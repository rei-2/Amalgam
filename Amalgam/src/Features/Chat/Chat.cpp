#include "Chat.h"
#include "../Configs/Configs.h"
#include "../Visuals/MarkSpot/MarkSpot.h"
#include <chrono>
#include <iomanip>
#include <atomic>
#include <ctime>
#include <fstream>
#include <cstdio>
#include <algorithm>
#include <random>

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#include <lmcons.h>  // For UNLEN
#include <wincrypt.h> // For CryptGenRandom
#endif

// Global chat instance pointer for HTTP logging
static CChat* g_ChatInstance = nullptr;

// Debug-only message macro
#ifdef _DEBUG
#define DEBUG_MSG(msg) do { if (g_ChatInstance) g_ChatInstance->QueueMessage("Matrix Debug", msg); } while(0)
#else
#define DEBUG_MSG(msg) do { } while(0)
#endif


CChat::CChat() : m_bInitialized(false)
{
    // Chat settings now handled by main config system
    
    // Set global instance pointer for HTTP logging
    g_ChatInstance = this;
    
    // Defer crypto and HTTP initialization until first use
    // This prevents crashes during DLL injection
}

void CChat::EnsureInitialized()
{
    if (m_bInitialized) return;
    
    try
    {
        // Initialize encryption system
        m_pCrypto = std::make_unique<MatrixCrypto>();
        
        // Set up HTTP logging callback to use TF2 console
        HttpClient::SetLogCallback([](const std::string& prefix, const std::string& message) {
            // Access the global chat instance to queue messages
            if (g_ChatInstance) {
                g_ChatInstance->QueueMessage(prefix, message);
            }
        });
        
        m_bInitialized = true;
    }
    catch (...)
    {
        // If initialization fails, don't crash
        m_bInitialized = false;
    }
}

CChat::~CChat()
{
    // Clear global instance pointer
    g_ChatInstance = nullptr;
    
    // Chat settings now handled by main config system
    Disconnect();
}

void CChat::Connect()
{
    if (m_bConnected.load() || m_bLoginInProgress.load())
        return;
    
    // Check if we have valid configuration
    if (Vars::Chat::Server.Value.empty() || 
        Vars::Chat::Username.Value.empty() || 
        Vars::Chat::Password.Value.empty())
    {
        std::lock_guard<std::mutex> lock(m_StatusMutex);
        m_sLastError = "Please configure server, username, and password first.";
        return;
    }
    
    m_bLoginInProgress.store(true);
    {
        std::lock_guard<std::mutex> lock(m_StatusMutex);
        m_sLastError = "";
    }
    
    // Start connection in a separate thread to avoid blocking the game
    std::thread connectThread([this]() {
        // Capture values to avoid potential race conditions
        std::string username = Vars::Chat::Username.Value;
        std::string password = Vars::Chat::Password.Value;
        
        // Initialize base URL
        m_sBaseUrl = "https://" + Vars::Chat::Server.Value;
        
        // Test basic connectivity first
#ifdef _DEBUG
        DEBUG_MSG("=== STARTING MATRIX REGISTRATION DEBUG ===");
        DEBUG_MSG("Server: " + Vars::Chat::Server.Value);
        DEBUG_MSG("Username: " + Vars::Chat::Username.Value);
        DEBUG_MSG("Testing connectivity to: " + m_sBaseUrl);
#endif
        auto versionResponse = HttpClient::Get(m_sBaseUrl + "/_matrix/client/versions", {});
        DEBUG_MSG("Server versions check status: " + std::to_string(versionResponse.status_code));
        
        if (versionResponse.status_code == 0)
        {
            DEBUG_MSG("Network connectivity failed: " + versionResponse.text);
            std::lock_guard<std::mutex> lock(m_StatusMutex);
            m_sLastError = "Cannot connect to server: " + versionResponse.text;
            m_bLoginInProgress.store(false);
            return;
        }
        
        // Try login first
        if (HttpLogin(username, password) && HttpJoinRoom())
        {
            m_bLoginInProgress.store(false);
            m_bConnected.store(true);
            
            QueueMessage("Matrix", "Connected to room '" + Vars::Chat::Room.Value + "' in space '" + Vars::Chat::Space.Value + "'");
            QueueMessage("Matrix", "Use !! prefix to send messages to Matrix chat");
            
            if (!m_ClientThread.joinable())
            {
                m_ClientThread = std::thread(&CChat::HandleMatrixEvents, this);
            }
        }
        else
        {
            // Check if login failed due to unknown user
            bool shouldCreateAccount = false;
            {
                std::lock_guard<std::mutex> lock(m_StatusMutex);
                shouldCreateAccount = (m_sLastError.find("Unknown user") != std::string::npos ||
                                     m_sLastError.find("M_USER_DEACTIVATED") != std::string::npos ||
                                     m_sLastError.find("M_UNKNOWN") != std::string::npos ||
                                     m_sLastError.find("not found") != std::string::npos);
            }
            
            if (shouldCreateAccount)
            {
                // User doesn't exist, try to create account
                QueueMessage("Matrix", "User not found, attempting to create account...");
                
                // For matrix.org, prefer email registration if email is provided
                if (Vars::Chat::Server.Value == "matrix.org" && !Vars::Chat::Email.Value.empty()) {
                    QueueMessage("Matrix", "Using email registration for matrix.org");
                    CreateAccountWithEmail(username, password, Vars::Chat::Email.Value);
                } else {
                    CreateAccount(username, password);
                }
            }
            else
            {
                // Other login error (wrong password, server error, etc.)
                m_bLoginInProgress.store(false);
                m_bConnected.store(false);
            }
        }
    });
    connectThread.detach();
}

void CChat::Disconnect()
{
    if (!m_bConnected.load() && !m_bLoginInProgress.load())
        return;
    
    // Signal thread to stop
    m_bShouldStop.store(true);
    m_bConnected.store(false);
    m_bLoginInProgress.store(false);
    
    // Wait for thread to finish
    if (m_ClientThread.joinable())
        m_ClientThread.join();
    
    // Reset stop flag for next connection
    m_bShouldStop.store(false);
    
    // Note: We intentionally do NOT reset registration state here
    // because the user might be in the middle of email verification
    // Registration state will be reset when starting a new registration
    // with different credentials or when registration completes
}

void CChat::CreateAccountWithEmail(const std::string& username, const std::string& password, const std::string& email)
{
    try
    {
        DEBUG_MSG("CreateAccountWithEmail called - Session: '" + m_sRegistrationSession + "'");
        DEBUG_MSG("Email verification state - Requested: " + std::string(m_bEmailVerificationRequested ? "true" : "false") + 
                    ", Completed: " + std::string(m_bEmailVerificationCompleted ? "true" : "false"));
        
        // Check if email verification was already requested for these exact credentials
        bool sameCredentials = m_sPendingUsername == username && 
                              m_sPendingPassword == password &&
                              m_sLastEmailRequested == email;
        
        bool hasActiveRegistration = !m_sRegistrationSession.empty() && sameCredentials;
        
        if (hasActiveRegistration && m_bEmailVerificationRequested) {
            DEBUG_MSG("Email verification already requested for these exact credentials");
            
            // If email verification is completed, try to complete registration
            if (m_bEmailVerificationCompleted) {
                DEBUG_MSG("Email verification completed - attempting registration");
                CreateAccountInternal(username, password, m_sRegistrationSession);
                return;
            }
            
            // Email already sent but not yet verified - don't send duplicate
            {
                std::lock_guard<std::mutex> lock(m_StatusMutex);
                m_sLastError = "";
                m_sLastSuccess = "Email verification already sent to " + email + ". Check your email and click the verification link, then try 'Create New Account' again.";
            }
            return;
        }
        
        // If credentials are different or no active session, reset and start fresh
        if (!sameCredentials || m_sRegistrationSession.empty()) {
            DEBUG_MSG("New registration attempt - resetting state");
            ResetRegistrationState();
            
            // Store new credentials
            m_sPendingUsername = username;
            m_sPendingPassword = password;
            m_sLastEmailRequested = email;
        } else {
            DEBUG_MSG("Continuing existing registration flow with same credentials");
        }
        
        // Only get fresh auth flows if we don't have an active session
        if (m_sRegistrationSession.empty()) {
            DEBUG_MSG("No active session - checking server capabilities");
            
            // First, check login flows to see if SSO is available (per Element approach)
            auto loginFlowResponse = HttpClient::Get(
                "https://" + Vars::Chat::Server.Value + "/_matrix/client/v3/login",
                {}
            );
            
            bool hasSsoFlow = false;
            std::string ssoUrl;
            
            if (loginFlowResponse.status_code == 200) {
                auto loginFlowJson = nlohmann::json::parse(loginFlowResponse.text);
                if (loginFlowJson.contains("flows")) {
                    for (const auto& flow : loginFlowJson["flows"]) {
                        if (flow.contains("type")) {
                            std::string flowType = flow["type"];
                            if (flowType == "m.login.sso" || flowType == "m.login.cas") {
                                hasSsoFlow = true;
                                DEBUG_MSG("Server supports SSO authentication");
                                // Build registration URL using the same OAuth2 flow as Element
                                if (Vars::Chat::Server.Value == "matrix.org") {
                                    // For matrix.org, use OAuth2 authorization with prompt=create
                                    std::string clientId = "01JR7KPNATR5JC45E4VN9BRA5G";  // Element's client ID
                                    std::string redirectUri = HttpClient::UrlEncode("https://app.element.io/");
                                    std::string responseType = "code";
                                    std::string scope = HttpClient::UrlEncode("openid urn:matrix:org.matrix.msc2967.client:api:* urn:matrix:org.matrix.msc2967.client:device:cMuq4TjeBc");
                                    
                                    ssoUrl = "https://account.matrix.org/authorize";
                                    ssoUrl += "?client_id=" + clientId;
                                    ssoUrl += "&redirect_uri=" + redirectUri;
                                    ssoUrl += "&response_type=" + responseType;
                                    ssoUrl += "&scope=" + scope;
                                    ssoUrl += "&prompt=create";  // This is the key - tells OAuth to show registration
                                    ssoUrl += "&response_mode=query";
                                } else {
                                    // For other servers, fall back to standard SSO approach
                                    std::string redirectUrl = "https://app.element.io/";
                                    ssoUrl = "https://" + Vars::Chat::Server.Value + "/_matrix/client/v3/login/sso/redirect";
                                    ssoUrl += "?redirectUrl=" + HttpClient::UrlEncode(redirectUrl);
                                    ssoUrl += "&org.matrix.msc3824.action=register";
                                }
                                break;
                            }
                        }
                    }
                }
            }
            
            // Now try registration request to get auth flows
            nlohmann::json flowRequest = {
                {"username", username},
                {"password", password},
                {"device_id", "AmalgamClient"},
                {"initial_device_display_name", "Amalgam TF2 Client"},
                {"inhibit_login", false}
            };
            
            auto flowResponse = HttpClient::Post(
                "https://" + Vars::Chat::Server.Value + "/_matrix/client/v3/register",
                flowRequest.dump(),
                {{"Content-Type", "application/json"}}
            );
            
            if (flowResponse.status_code == 401)
            {
                // Parse the auth flows response
                auto flowJson = nlohmann::json::parse(flowResponse.text);
                if (flowJson.contains("session"))
                {
                    m_sRegistrationSession = flowJson["session"];
                    DEBUG_MSG("Got new session: " + m_sRegistrationSession);
                }
                
                if (flowJson.contains("flows")) {
                    DEBUG_MSG("Available auth flows: " + flowJson["flows"].dump());
                    
                    // Check what flows are available
                    bool hasEmailIdentity = false;
                    bool hasRecaptcha = false;
                    bool hasTerms = false;
                    bool hasPassword = false;
                    
                    for (const auto& flow : flowJson["flows"]) {
                        if (flow.contains("stages")) {
                            for (const auto& stage : flow["stages"]) {
                                std::string stageType = stage;
                                if (stageType == "m.login.email.identity") hasEmailIdentity = true;
                                if (stageType == "m.login.recaptcha") hasRecaptcha = true;
                                if (stageType == "m.login.terms") hasTerms = true;
                                if (stageType == "m.login.password") hasPassword = true;
                            }
                        }
                    }
                    
                    DEBUG_MSG("Auth flow support - Email: " + std::string(hasEmailIdentity ? "yes" : "no") + 
                                                ", ReCaptcha: " + std::string(hasRecaptcha ? "yes" : "no") +
                                                ", Terms: " + std::string(hasTerms ? "yes" : "no") +
                                                ", Password: " + std::string(hasPassword ? "yes" : "no"));
                    
                    // Matrix.org typically requires both email AND recaptcha
                    if (hasRecaptcha && !hasEmailIdentity) {
                        {
                            std::lock_guard<std::mutex> lock(m_StatusMutex);
                            m_sLastError = "Matrix.org requires reCAPTCHA verification. Please register manually at https://app.element.io";
                        }
                        return;
                    }
                }
            } else if (flowResponse.status_code == 200) {
                // Registration succeeded without additional auth
                CompleteRegistration(flowResponse.text);
                ResetRegistrationState();
                return;
            } else if (flowResponse.status_code == 403) {
                DEBUG_MSG("Registration forbidden (403): " + flowResponse.text);
                
                // Use previously discovered SSO info
                if (hasSsoFlow) {
                    QueueMessage("Matrix", "Server requires external registration. Opening browser...");
                    
                    #ifdef _WIN32
                        // Windows: use ShellExecute to open URL in default browser
                        ShellExecuteA(NULL, "open", ssoUrl.c_str(), NULL, NULL, SW_SHOWNORMAL);
                    #elif __linux__
                        // Linux: use xdg-open
                        system(("xdg-open \"" + ssoUrl + "\"").c_str());
                    #elif __APPLE__
                        // macOS: use open command
                        system(("open \"" + ssoUrl + "\"").c_str());
                    #endif
                    
                    {
                        std::lock_guard<std::mutex> lock(m_StatusMutex);
                        m_sLastError = "Opened browser for external registration. Please register there, then try connecting again.";
                    }
                } else {
                    {
                        std::lock_guard<std::mutex> lock(m_StatusMutex);
                        m_sLastError = "Registration disabled on this server. Try a different homeserver or contact the admin.";
                    }
                }
                return;
            } else if (flowResponse.status_code == 429) {
                DEBUG_MSG("Rate limited (429): " + flowResponse.text);
                {
                    std::lock_guard<std::mutex> lock(m_StatusMutex);
                    m_sLastError = "Too many registration attempts. Please wait and try again later.";
                }
                return;
            } else {
                DEBUG_MSG("=== REGISTRATION FAILED ===");
                DEBUG_MSG("HTTP Status: " + std::to_string(flowResponse.status_code));
                DEBUG_MSG("Response Headers: " + (flowResponse.text.empty() ? std::string("[EMPTY]") : std::string("[PRESENT]")));
                DEBUG_MSG("Full Response: " + flowResponse.text);
                DEBUG_MSG("URL Used: https://" + Vars::Chat::Server.Value + "/_matrix/client/v3/register");
                {
                    std::lock_guard<std::mutex> lock(m_StatusMutex);
                    m_sLastError = "Failed to start registration with server: " + std::to_string(flowResponse.status_code) + " - " + flowResponse.text;
                }
                return;
            }
        } else {
            DEBUG_MSG("Using existing session: " + m_sRegistrationSession);
        }
        
        // Only request email verification if not already requested for this session
        if (!m_bEmailVerificationRequested) {
            DEBUG_MSG("Requesting email verification for the first time");
            
            // Ensure we have a valid session before requesting email verification
            if (m_sRegistrationSession.empty()) {
                DEBUG_MSG("Error: No registration session available for email verification");
                {
                    std::lock_guard<std::mutex> lock(m_StatusMutex);
                    m_sLastError = "Registration session error. Please try again.";
                }
                return;
            }
            
            // Generate unique client secret for this registration session
            m_sClientSecret = "AmalgamSecret" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count());
            
            nlohmann::json emailRequestData = {
                {"client_secret", m_sClientSecret},
                {"email", email},
                {"send_attempt", 1}
            };
            
            // Try homeserver's identity server endpoint first (spec compliant v2 API)
            auto emailResponse = HttpClient::Post(
                "https://" + Vars::Chat::Server.Value + "/_matrix/identity/v2/validate/email/requestToken",
                emailRequestData.dump(),
                {{"Content-Type", "application/json"}}
            );
            
            bool usedHomeserverIdentity = (emailResponse.status_code == 200);
            
            if (!usedHomeserverIdentity) {
                // Try Matrix.org's identity server endpoint (v2 API)
                emailResponse = HttpClient::Post(
                    "https://vector.im/_matrix/identity/v2/validate/email/requestToken",
                    emailRequestData.dump(),
                    {{"Content-Type", "application/json"}}
                );
            }
            
            if (emailResponse.status_code == 200) {
                auto emailJson = nlohmann::json::parse(emailResponse.text);
                m_sEmailSid = emailJson["sid"];
                
                // Track which identity server provided the SID
                if (usedHomeserverIdentity) {
                    m_sIdentityServer = Vars::Chat::Server.Value;
                } else {
                    m_sIdentityServer = "vector.im";
                }
                
                // Mark email verification as requested
                m_bEmailVerificationRequested = true;
                
                {
                    std::lock_guard<std::mutex> lock(m_StatusMutex);
                    m_sLastError = "";
                    m_sLastSuccess = "Email verification sent to " + email + ". 1) Click the link in your email 2) After seeing 'Verification successful', click 'Create New Account' again.";
                }
                QueueMessage("Matrix", "Email verification sent to " + email);
                DEBUG_MSG("Email SID: " + m_sEmailSid + ", Identity Server: " + m_sIdentityServer + ", Client Secret: " + m_sClientSecret);
                
            } else {
                DEBUG_MSG("Email request failed: " + std::to_string(emailResponse.status_code));
                {
                    std::lock_guard<std::mutex> lock(m_StatusMutex);
                    m_sLastError = "Failed to send email verification. Please try again or use a different email.";
                }
                return;
            }
        } else {
            DEBUG_MSG("Email verification already requested - skipping duplicate request");
            {
                std::lock_guard<std::mutex> lock(m_StatusMutex);
                m_sLastError = "";
                m_sLastSuccess = "Email verification already sent. Check your email and click the verification link, then try again.";
            }
        }
        
        DEBUG_MSG("Email registration prepared - waiting for email verification");
        
    }
    catch (const std::exception& e)
    {
        std::lock_guard<std::mutex> lock(m_StatusMutex);
        m_sLastError = "Email registration error: " + std::string(e.what());
        DEBUG_MSG("Email registration exception: " + std::string(e.what()));
    }
}

void CChat::SubmitEmailVerification(const std::string& token)
{
    try
    {
        DEBUG_MSG("Session check - Session: '" + m_sRegistrationSession + "', Username: '" + m_sPendingUsername + "'");
        DEBUG_MSG("Email SID: '" + m_sEmailSid + "', Client Secret: '" + m_sClientSecret + "'");
        
        // If session is empty, user might need to restart email registration
        if (m_sRegistrationSession.empty()) {
            DEBUG_MSG("Session empty - email verification may have expired");
            {
                std::lock_guard<std::mutex> lock(m_StatusMutex);
                m_sLastError = "Email verification session expired. Please try 'Create New Account' again.";
            }
            return;
        }
        
        if (m_sRegistrationSession.empty() || m_sPendingUsername.empty() || m_sEmailSid.empty() || m_sClientSecret.empty())
        {
            std::lock_guard<std::mutex> lock(m_StatusMutex);
            m_sLastError = "No pending email verification session - Session: " + 
                          (m_sRegistrationSession.empty() ? std::string("empty") : std::string("present")) + 
                          ", Username: " + 
                          (m_sPendingUsername.empty() ? std::string("empty") : std::string("present")) +
                          ", EmailSID: " + 
                          (m_sEmailSid.empty() ? std::string("empty") : std::string("present")) +
                          ", ClientSecret: " + 
                          (m_sClientSecret.empty() ? std::string("empty") : std::string("present"));
            return;
        }
        
        // According to Matrix spec, after clicking email link, just retry registration
        // The token field is often not actually needed - the email click validates it
        DEBUG_MSG("Proceeding with registration after email verification");
        
        // Mark email verification as completed
        m_bEmailVerificationCompleted = true;
        
        {
            std::lock_guard<std::mutex> lock(m_StatusMutex);
            m_sLastError = "";
            m_sLastSuccess = "Processing email verification...";
        }
        
        // Now proceed with registration using the validated email
        nlohmann::json registerData = {
            {"username", m_sPendingUsername},
            {"password", m_sPendingPassword},
            {"device_id", "AmalgamClient"},
            {"initial_device_display_name", "Amalgam TF2 Client"},
            {"inhibit_login", false},
            {"auth", {
                {"type", "m.login.email.identity"},
                {"threepid_creds", {
                    {"client_secret", m_sClientSecret},
                    {"id_server", m_sIdentityServer},
                    {"sid", m_sEmailSid}
                }},
                {"session", m_sRegistrationSession}
            }}
        };
        
        DEBUG_MSG("Submitting registration with validated email");
        
        auto response = HttpClient::Post(
            "https://" + Vars::Chat::Server.Value + "/_matrix/client/v3/register",
            registerData.dump(),
            {{"Content-Type", "application/json"}}
        );
        
        if (response.status_code == 200)
        {
            CompleteRegistration(response.text);
            
            // Clear all registration session data
            ResetRegistrationState();
        }
        else
        {
            HandleRegistrationError(response.status_code, response.text);
            
            // Show error in UI status
            std::lock_guard<std::mutex> lock(m_StatusMutex);
            if (m_sLastError.empty()) {
                m_sLastError = "Email verification failed - please try again";
            }
        }
    }
    catch (const std::exception& e)
    {
        std::lock_guard<std::mutex> lock(m_StatusMutex);
        m_sLastError = "Email verification error: " + std::string(e.what());
    }
}

void CChat::CreateAccountInternal(const std::string& username, const std::string& password, const std::string& sessionId)
{
    try
    {
        // Create registration data
        nlohmann::json registerData = {
            {"username", username},
            {"password", password},
            {"device_id", "AmalgamClient"},
            {"initial_device_display_name", "Amalgam TF2 Client"},
            {"inhibit_login", false}
        };
        
        if (!sessionId.empty())
        {
            registerData["auth"] = {
                {"type", "m.login.password"},
                {"user", username},
                {"password", password},
                {"session", sessionId}
            };
        }
        
        auto response = HttpClient::Post(
            "https://" + Vars::Chat::Server.Value + "/_matrix/client/v3/register",
            registerData.dump(),
            {{"Content-Type", "application/json"}}
        );
        
        if (response.status_code == 200)
        {
            CompleteRegistration(response.text);
        }
        else if (response.status_code == 401)
        {
            HandleRegistrationError(response.status_code, response.text);
        }
        else
        {
            HandleRegistrationError(response.status_code, response.text);
        }
    }
    catch (const std::exception& e)
    {
        std::lock_guard<std::mutex> lock(m_StatusMutex);
        m_sLastError = "Account creation error: " + std::string(e.what());
    }
}

void CChat::CreateAccount(const std::string& username, const std::string& password)
{
    try
    {
        DEBUG_MSG("=== STARTING REGULAR ACCOUNT CREATION ===");
        printf("[DIRECT] Starting Matrix account creation for user: %s\n", username.c_str());
        fflush(stdout);
        DEBUG_MSG("Server: " + Vars::Chat::Server.Value);
        DEBUG_MSG("Username: " + username);
        
        // First, get the available auth flows from the server
        nlohmann::json initialRequest = {
            {"username", username},
            {"password", password},
            {"device_id", "AmalgamClient"},
            {"initial_device_display_name", "Amalgam TF2 Client"},
            {"inhibit_login", false}
        };
        
        auto flowResponse = HttpClient::Post(
            "https://" + Vars::Chat::Server.Value + "/_matrix/client/v3/register",
            initialRequest.dump(),
            {{"Content-Type", "application/json"}}
        );
        
        std::string sessionId;
        nlohmann::json flows;
        
        if (flowResponse.status_code == 401)
        {
            // Parse the auth flows response
            auto flowJson = nlohmann::json::parse(flowResponse.text);
            if (flowJson.contains("session"))
            {
                sessionId = flowJson["session"];
            }
            if (flowJson.contains("flows")) {
                flows = flowJson["flows"];
            }
        }
        else if (flowResponse.status_code == 200)
        {
            // Registration succeeded without additional auth
            CompleteRegistration(flowResponse.text);
            return;
        }
        
        // Try to register with appropriate auth based on available flows
        nlohmann::json registerData = {
            {"username", username},
            {"password", password},
            {"device_id", "AmalgamClient"},
            {"initial_device_display_name", "Amalgam TF2 Client"},
            {"inhibit_login", false}
        };
        
        if (!sessionId.empty())
        {
            // Check if we need password auth or can use dummy auth
            bool needsPasswordAuth = false;
            bool canUseDummy = false;
            
            for (const auto& flow : flows) {
                if (flow.contains("stages")) {
                    for (const auto& stage : flow["stages"]) {
                        std::string stageType = stage;
                        if (stageType == "m.login.password") needsPasswordAuth = true;
                        if (stageType == "m.login.dummy") canUseDummy = true;
                    }
                }
            }
            
            if (needsPasswordAuth) {
                registerData["auth"] = {
                    {"type", "m.login.password"},
                    {"user", username},
                    {"password", password},
                    {"session", sessionId}
                };
            }
            else if (canUseDummy) {
                registerData["auth"] = {
                    {"type", "m.login.dummy"},
                    {"session", sessionId}
                };
            }
            else {
                // No supported auth flow found
                std::lock_guard<std::mutex> lock(m_StatusMutex);
                m_sLastError = "No supported authentication flow available";
                return;
            }
        }
        
        auto response = HttpClient::Post(
            "https://" + Vars::Chat::Server.Value + "/_matrix/client/v3/register",
            registerData.dump(),
            {{"Content-Type", "application/json"}}
        );
        
        if (response.status_code == 200)
        {
            // Registration successful with login
            auto json = nlohmann::json::parse(response.text);
            m_sAccessToken = json["access_token"];
            
            // Set connected state first
            m_bLoginInProgress.store(false);
            m_bConnected.store(true);
            
            // Update status message without holding lock during other operations
            {
                std::lock_guard<std::mutex> lock(m_StatusMutex);
                m_sLastError = "";
                m_sLastSuccess = "Account created successfully! You are now logged in.";
            }
            
            // Clear any pending registration state
            ResetRegistrationState();
            
            // Join room and start sync (outside of mutex lock)
            if (HttpJoinRoom())
            {
                QueueMessage("Matrix", "Account created and connected to room '" + Vars::Chat::Room.Value + "' in space '" + Vars::Chat::Space.Value + "'");
                QueueMessage("Matrix", "Use !! prefix to send messages to Matrix chat");
                
                // Initialize encryption after successful connection
                SDK::Output("Matrix", "Calling InitializeEncryption() after successful login", { 200, 255, 200, 255 });
                InitializeEncryption();
                
                if (!m_ClientThread.joinable())
                {
                    m_ClientThread = std::thread(&CChat::HandleMatrixEvents, this);
                }
            }
        }
        else if (response.status_code == 400)
        {
            auto json = nlohmann::json::parse(response.text);
            std::lock_guard<std::mutex> lock(m_StatusMutex);
            if (json.contains("errcode") && json["errcode"] == "M_USER_IN_USE")
            {
                m_sLastError = "Username already taken. Try logging in instead.";
            }
            else
            {
                m_sLastError = "Account creation failed: " + json["error"].get<std::string>();
            }
        }
        else if (response.status_code == 403)
        {
            std::lock_guard<std::mutex> lock(m_StatusMutex);
            m_sLastError = "Registration disabled on this server. Contact server admin.";
        }
        else
        {
            auto json = nlohmann::json::parse(response.text);
            std::lock_guard<std::mutex> lock(m_StatusMutex);
            if (json.contains("error"))
            {
                m_sLastError = "Account creation failed: " + json["error"].get<std::string>();
            }
            else if (json.contains("errcode"))
            {
                m_sLastError = "Account creation failed: " + json["errcode"].get<std::string>();
            }
            else
            {
                m_sLastError = "Account creation failed with status: " + std::to_string(response.status_code);
            }
        }
    }
    catch (const std::exception& e)
    {
        // Reset connection state on error
        m_bLoginInProgress.store(false);
        m_bConnected.store(false);
        
        std::lock_guard<std::mutex> lock(m_StatusMutex);
        m_sLastError = "Account creation error: " + std::string(e.what());
    }
}

void CChat::ShowRecaptcha()
{
    if (m_sRecaptchaPublicKey.empty()) {
        std::lock_guard<std::mutex> lock(m_StatusMutex);
        m_sLastError = "No reCAPTCHA public key available";
        return;
    }
    
    // Simple visual reCAPTCHA challenge - this is a placeholder for a real implementation
    // In a real application, you would integrate with an actual reCAPTCHA service
    std::lock_guard<std::mutex> lock(m_StatusMutex);
    m_sLastSuccess = "Complete the visual challenge below, then click 'Complete Registration'";
    m_bRecaptchaRequired = true;
    m_bRecaptchaShowing = true;
}

void CChat::CompleteRecaptcha()
{
    if (m_sRegistrationSession.empty()) {
        std::lock_guard<std::mutex> lock(m_StatusMutex);
        m_sLastError = "No active registration session";
        return;
    }
    
    // Mark reCAPTCHA as completed
    m_bRecaptchaRequired = false;
    m_bRecaptchaShowing = false;
    
    {
        std::lock_guard<std::mutex> lock(m_StatusMutex);
        m_sLastSuccess = "reCAPTCHA completed! Retrying registration...";
    }
    
    // Automatically retry registration with session
    CreateAccountInternal(m_sPendingUsername, m_sPendingPassword, m_sRegistrationSession);
}


std::string CChat::GetStatus() const
{
    std::lock_guard<std::mutex> lock(m_StatusMutex);
    
    // Return error first if there is one, otherwise return success message
    if (!m_sLastError.empty()) {
        return m_sLastError;
    }
    
    if (!m_sLastSuccess.empty()) {
        return m_sLastSuccess;
    }
    
    // Default status
    return "Ready";
}


void CChat::CompleteRegistration(const std::string& response_text)
{
    // Registration successful with login
    auto json = nlohmann::json::parse(response_text);
    m_sAccessToken = json["access_token"];
    
    // Set connected state first
    m_bLoginInProgress.store(false);
    m_bConnected.store(true);
    
    // Update status message without holding lock during other operations
    {
        std::lock_guard<std::mutex> lock(m_StatusMutex);
        m_sLastError = "";
        m_sLastSuccess = "Account created successfully! You are now logged in.";
    }
    
    // Join room and start sync (outside of mutex lock)
    if (HttpJoinRoom())
    {
        QueueMessage("Matrix", "Account created and connected to room '" + Vars::Chat::Room.Value + "' in space '" + Vars::Chat::Space.Value + "'");
        QueueMessage("Matrix", "Use !! prefix to send messages to Matrix chat");
        
        // Initialize encryption after successful connection
        SDK::Output("Matrix", "Calling InitializeEncryption() after room join", { 200, 255, 200, 255 });
        InitializeEncryption();
        
        if (!m_ClientThread.joinable())
        {
            m_ClientThread = std::thread(&CChat::HandleMatrixEvents, this);
        }
    }
}

void CChat::HandleRegistrationError(int status_code, const std::string& response_text)
{
    try
    {
        auto json = nlohmann::json::parse(response_text);
        std::string debugMessage;
        
        {
            std::lock_guard<std::mutex> lock(m_StatusMutex);
            
            if (status_code == 400)
            {
                if (json.contains("errcode") && json["errcode"] == "M_USER_IN_USE")
                {
                    m_sLastError = "Username already taken. Try logging in instead.";
                    // Reset registration state on username conflict
                    ResetRegistrationState();
                }
                else if (json.contains("error"))
                {
                    m_sLastError = "Account creation failed: " + json["error"].get<std::string>();
                }
                else
                {
                    m_sLastError = "Invalid registration data";
                }
            }
            else if (status_code == 403)
            {
                m_sLastError = "Registration disabled on this server. Contact server admin.";
            }
            else if (status_code == 429)
            {
                m_sLastError = "Too many registration attempts. Please wait and try again.";
            }
            else if (status_code == 401)
            {
                // Check for auth flows including reCAPTCHA
                if (json.contains("session")) {
                    m_sRegistrationSession = json["session"];
                }
                
                // Check if reCAPTCHA is required
                bool needsRecaptcha = false;
                if (json.contains("flows")) {
                    for (const auto& flow : json["flows"]) {
                        if (flow.contains("stages")) {
                            for (const auto& stage : flow["stages"]) {
                                std::string stageType = stage.get<std::string>();
                                if (stageType == "m.login.recaptcha") needsRecaptcha = true;
                            }
                        }
                    }
                }
                
                if (needsRecaptcha) {
                    m_bRecaptchaRequired = true;
                    
                    // Extract reCAPTCHA public key from the response
                    if (json.contains("params") && json["params"].contains("m.login.recaptcha") && 
                        json["params"]["m.login.recaptcha"].contains("public_key")) {
                        m_sRecaptchaPublicKey = json["params"]["m.login.recaptcha"]["public_key"];
                        debugMessage = "reCAPTCHA public key extracted: " + m_sRecaptchaPublicKey;
                    }
                    
                    m_sLastError = "";
                    m_sLastSuccess = "Matrix.org requires reCAPTCHA verification. Complete the challenge below.";
                } else {
                    if (json.contains("error")) {
                        m_sLastError = "Registration failed: " + json["error"].get<std::string>();
                    } else {
                        m_sLastError = "Authentication required - unknown flow";
                    }
                }
            }
            else
            {
                if (json.contains("error"))
                {
                    m_sLastError = "Registration failed: " + json["error"].get<std::string>();
                }
                else if (json.contains("errcode"))
                {
                    m_sLastError = "Registration failed: " + json["errcode"].get<std::string>();
                }
                else
                {
                    m_sLastError = "Registration failed with status: " + std::to_string(status_code);
                }
            }
        }
        
        // Send debug message outside of mutex lock
        if (!debugMessage.empty()) {
            DEBUG_MSG(debugMessage);
        }
    }
    catch (...)
    {
        std::lock_guard<std::mutex> lock(m_StatusMutex);
        m_sLastError = "Registration failed with status: " + std::to_string(status_code);
    }
}

void CChat::ResetRegistrationState()
{
    // Clear all registration session data
    m_sRegistrationSession.clear();
    m_sPendingUsername.clear();
    m_sPendingPassword.clear();
    m_sClientSecret.clear();
    m_sEmailSid.clear();
    m_sIdentityServer.clear();
    m_sRecaptchaPublicKey.clear();
    m_sLastEmailRequested.clear();
    
    // Reset state flags
    m_bEmailVerificationRequested = false;
    m_bEmailVerificationCompleted = false;
    m_bRecaptchaRequired = false;
    m_bRecaptchaShowing = false;
    
    QueueMessage("Matrix Debug", "Registration state reset");
}

bool CChat::IsEmailVerificationPending() const
{
    return m_bEmailVerificationRequested && !m_bEmailVerificationCompleted && 
           !m_sRegistrationSession.empty() && !m_sEmailSid.empty();
}


void CChat::SendMessage(const std::string& message)
{
    EnsureInitialized();
    
    if (!m_bConnected.load() || message.empty())
        return;
    
    // Send message in a separate thread to avoid blocking the game
    std::thread sendThread([this, message]() {
        HttpSendMessage(message);
    });
    sendThread.detach();
}

bool CChat::ProcessGameChatMessage(const std::string& message)
{
    QueueMessage("Matrix Debug", "Processing message: '" + message + "'");
    
    // Trim leading whitespace and quotes first
    std::string trimmedMessage = message;
    while (!trimmedMessage.empty() && (trimmedMessage.front() == ' ' || trimmedMessage.front() == '\t' || trimmedMessage.front() == '"'))
        trimmedMessage.erase(0, 1);
    while (!trimmedMessage.empty() && (trimmedMessage.back() == ' ' || trimmedMessage.back() == '\t' || trimmedMessage.back() == '"'))
        trimmedMessage.erase(trimmedMessage.length() - 1, 1);
    
    QueueMessage("Matrix Debug", "After trimming: '" + trimmedMessage + "'");
    
    // Check if message starts with !! prefix
    if (trimmedMessage.length() >= 2 && trimmedMessage.substr(0, 2) == "!!")
    {
        DEBUG_MSG("Found !! prefix, processing Matrix message");
        
        std::string chatMessage = trimmedMessage.substr(2); // Remove !! prefix
        
        // Trim whitespace
        while (!chatMessage.empty() && (chatMessage.front() == ' ' || chatMessage.front() == '\t'))
            chatMessage.erase(0, 1);
        
        if (!chatMessage.empty())
        {
            DEBUG_MSG("Sending: " + chatMessage);
            
            // Check if we're connected before sending
            if (!m_bConnected.load())
            {
                QueueMessage("Matrix", "Not connected to Matrix server");
                return false; // Allow message to pass through to game chat when not connected
            }
            
            SendMessage(chatMessage);
            DEBUG_MSG("Blocking message from game chat");
            return true; // Message was processed and should be blocked from game chat
        }
        else
        {
            DEBUG_MSG("Empty message after trimming, blocking from game chat");
            return true; // Block empty !! messages from game chat
        }
    }
    
    QueueMessage("Matrix Debug", "No !! prefix, allowing normal chat");
    return false; // Message was not processed
}

void CChat::ProcessIncomingMessage(const std::string& sender, const std::string& content)
{
    // Extract username from Matrix user ID (@username:server.com)
    std::string senderName = sender;
    if (sender.starts_with("@"))
    {
        auto colonPos = sender.find(':');
        if (colonPos != std::string::npos)
        {
            senderName = sender.substr(1, colonPos - 1); // Remove @ and :server part
        }
    }
    
    // Check if this is a MarkSpot message and filter it out
    if (content.starts_with("!MARK:"))
    {
        // Forward to MarkSpot feature for processing
        F::MarkSpot.ProcessMatrixMessage(senderName, content);
        return; // Don't display in chat
    }
    
    // Show all messages including our own (for confirmation that they were sent)
    // Queue the message for display on the main thread instead of calling ChatPrintf directly
    // ChatPrintf is not thread-safe and crashes when called from background threads
    QueueMessage("[Matrix] " + senderName, content);
}

void CChat::QueueMessage(const std::string& sender, const std::string& content)
{
    std::lock_guard<std::mutex> lock(m_MessageQueueMutex);
    m_MessageQueue.push({sender, content});
}

void CChat::ProcessQueuedMessages()
{
    // Handle auto-connect after config is loaded (only once)
    if (!m_bAutoConnectAttempted.load() && F::Configs.m_bConfigLoaded)
    {
        m_bAutoConnectAttempted.store(true);
        
        // Auto-connect if enabled
        if (Vars::Chat::AutoConnect.Value)
        {
            Connect();
        }
    }
    
    std::lock_guard<std::mutex> lock(m_MessageQueueMutex);
    while (!m_MessageQueue.empty())
    {
        auto message = m_MessageQueue.front();
        m_MessageQueue.pop();
        DisplayInGameMessage(message.sender, message.content);
    }
}

void CChat::DisplayInGameMessage(const std::string& sender, const std::string& content)
{
    // Handle debug messages
    if (sender.find("Debug") != std::string::npos)
    {
#ifdef _DEBUG
        // Debug messages only go to console when debug is enabled
        if (I::CVar)
        {
            Color_t tColor = { 100, 100, 100, 255 }; // Gray color for debug messages
            I::CVar->ConsoleColorPrintf(tColor, "[%s] %s\n", sender.c_str(), content.c_str());
        }
#endif
        return;
    }
    
    // Handle Matrix chat messages - try in-game chat first, fallback to console
    if (I::ClientModeShared && I::ClientModeShared->m_pChatElement)
    {
        try
        {
            // Extract just the username from sender (remove [Matrix] prefix if present)
            std::string userName = sender;
            if (userName.find("[Matrix] ") == 0)
            {
                userName = userName.substr(9); // Remove "[Matrix] " prefix
            }
            
            // Sanitize strings to prevent format issues and null bytes
            std::replace(userName.begin(), userName.end(), '%', '_');
            userName.erase(std::remove(userName.begin(), userName.end(), '\0'), userName.end());
            
            std::string safeContent = content;
            std::replace(safeContent.begin(), safeContent.end(), '%', '_');
            safeContent.erase(std::remove(safeContent.begin(), safeContent.end(), '\0'), safeContent.end());
            
            // Build the properly formatted message with TF2 color codes
            // \x7 followed by 6 hex chars for custom color (cyan for Matrix)
            // \x3 for username color (default player name color)  
            // \x1 for normal white text and IMPORTANT: terminates color formatting
            std::string formattedMessage;
            
            if (Vars::Chat::ShowTimestamps.Value)
            {
                // Get current time and format as [HH:MM:SS]
                auto now = std::chrono::system_clock::now();
                auto time_t = std::chrono::system_clock::to_time_t(now);
                struct tm tm;
                localtime_s(&tm, &time_t);
                
                std::string timestamp = std::format("[{:02d}:{:02d}:{:02d}] ", tm.tm_hour, tm.tm_min, tm.tm_sec);
                
                formattedMessage = std::format(
                    "\x7""00FFFF{}\x7""00FFFF[Matrix] \x3{}\x1: {}\x1", 
                    timestamp, userName, safeContent
                );
            }
            else
            {
                formattedMessage = std::format(
                    "\x7""00FFFF[Matrix] \x3{}\x1: {}\x1", 
                    userName, safeContent
                );
            }
            
            // Use ChatPrintf with single string parameter (no variadic args)
            // This matches the working pattern from SDK.cpp
            I::ClientModeShared->m_pChatElement->ChatPrintf(0, formattedMessage.c_str());
            return;
        }
        catch (...)
        {
            // If ChatPrintf fails, fall through to console
        }
    }
    
    // Fallback to console - only in debug builds to reduce console spam
#ifdef _DEBUG
    if (I::CVar)
    {
        Color_t tColor = { 0, 255, 255, 255 }; // Cyan color for Matrix messages
        I::CVar->ConsoleColorPrintf(tColor, "%s: %s\n", sender.c_str(), content.c_str());
    }
#endif
    // with more robust error handling and interface validation
}

void CChat::HandleMatrixEvents()
{
    while (m_bConnected.load() && !m_bShouldStop.load())
    {
        HttpSync();
        
        // Sleep briefly between syncs (since we use shorter timeout now)
        for (int i = 0; i < 10 && !m_bShouldStop.load(); ++i)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

void CChat::DrawConnectionStatus()
{
    // Connection status indicator
    if (m_bConnected.load())
    {
        ImGui::PushStyleColor(ImGuiCol_Text, { 0.0f, 1.0f, 0.0f, 1.0f }); // Green
        ImGui::Text("Status: Connected");
    }
    else if (m_bLoginInProgress.load())
    {
        ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 1.0f, 0.0f, 1.0f }); // Yellow
        ImGui::Text("Status: Connecting...");
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 0.0f, 0.0f, 1.0f }); // Red
        ImGui::Text("Status: Disconnected");
    }
    ImGui::PopStyleColor();
    
    std::string errorText, successText;
    {
        std::lock_guard<std::mutex> lock(m_StatusMutex);
        errorText = m_sLastError;
        successText = m_sLastSuccess;
    }
    
    if (!errorText.empty())
    {
        // Display error on a new line with proper wrapping
        ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 0.4f, 0.4f, 1.0f }); // Light red
        ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ImGui::GetContentRegionAvail().x);
        ImGui::TextWrapped("Error: %s", errorText.c_str());
        ImGui::PopTextWrapPos();
        ImGui::PopStyleColor();
    }
    
    if (!successText.empty())
    {
        // Display success message on a new line with proper wrapping
        ImGui::PushStyleColor(ImGuiCol_Text, { 0.4f, 1.0f, 0.4f, 1.0f }); // Light green
        ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ImGui::GetContentRegionAvail().x);
        ImGui::TextWrapped("%s", successText.c_str());
        ImGui::PopTextWrapPos();
        ImGui::PopStyleColor();
    }
}

// Matrix HTTP API implementations
bool CChat::HttpLogin(const std::string& username, const std::string& password)
{
    try
    {
        nlohmann::json loginData = {
            {"type", "m.login.password"},
            {"user", username},
            {"password", password}
        };
        
        std::map<std::string, std::string> headers = {
            {"Content-Type", "application/json"}
        };
        
        auto response = HttpClient::Post(
            m_sBaseUrl + "/_matrix/client/v3/login",
            loginData.dump(),
            headers
        );
        
        if (response.status_code == 200)
        {
            auto json = nlohmann::json::parse(response.text);
            m_sAccessToken = json["access_token"];
            DEBUG_MSG("Login successful, access token obtained");
            return true;
        }
        else
        {
            std::lock_guard<std::mutex> lock(m_StatusMutex);
            if (response.status_code == 0)
            {
                m_sLastError = "Network connection failed during login: " + response.text;
                DEBUG_MSG("Login failed with network error (status 0)");
                DEBUG_MSG("Error details: " + response.text);
            }
            else
            {
                try {
                    auto json = nlohmann::json::parse(response.text);
                    m_sLastError = "Login failed: " + json["error"].get<std::string>();
                } catch (...) {
                    m_sLastError = "Login failed with status " + std::to_string(response.status_code) + ": " + response.text;
                }
            }
            return false;
        }
    }
    catch (const std::exception& e)
    {
        std::lock_guard<std::mutex> lock(m_StatusMutex);
        m_sLastError = "Login error: " + std::string(e.what());
        return false;
    }
}

bool CChat::HttpJoinRoom()
{
    try
    {
        std::map<std::string, std::string> headers = {
            {"Authorization", "Bearer " + m_sAccessToken},
            {"Content-Type", "application/json"}
        };
        
        // Ensure base URL is set (in case of race condition)
        if (m_sBaseUrl.empty())
        {
            m_sBaseUrl = "https://" + Vars::Chat::Server.Value;
        }
        
        DEBUG_MSG("Starting room join process...");
        DEBUG_MSG("Space: " + Vars::Chat::Space.Value + ", Room: " + Vars::Chat::Room.Value + ", Server: " + Vars::Chat::Server.Value);
        DEBUG_MSG("Base URL: " + m_sBaseUrl);
        
        // Test basic HTTP connectivity first
        DEBUG_MSG("Testing HTTP connectivity...");
        auto testResponse = HttpClient::Get(m_sBaseUrl + "/_matrix/client/versions", {});
        DEBUG_MSG("Connectivity test result: " + std::to_string(testResponse.status_code));
        DEBUG_MSG("Connectivity test response: " + testResponse.text);
        
        // Strategy 1: Try to resolve and join the space first using directory lookup
        std::string spaceAlias = "#" + Vars::Chat::Space.Value + ":" + Vars::Chat::Server.Value;
        std::string encodedSpaceAlias = HttpClient::UrlEncode(spaceAlias);
        std::string spaceDirectoryUrl = m_sBaseUrl + "/_matrix/client/v3/directory/room/" + encodedSpaceAlias;
        
        DEBUG_MSG("Looking up space in directory: " + spaceAlias);
        DEBUG_MSG("Encoded alias: " + encodedSpaceAlias);
        DEBUG_MSG("Full URL: " + spaceDirectoryUrl);
        
        auto spaceDirectoryResponse = HttpClient::Get(spaceDirectoryUrl, headers);
        
        DEBUG_MSG("Space directory response code: " + std::to_string(spaceDirectoryResponse.status_code));
        DEBUG_MSG("Space directory response: " + spaceDirectoryResponse.text);
        
        if (spaceDirectoryResponse.status_code == 200)
        {
            auto spaceDirectoryJson = nlohmann::json::parse(spaceDirectoryResponse.text);
            std::string spaceRoomId = spaceDirectoryJson["room_id"];
            DEBUG_MSG("Found space in directory: " + spaceRoomId);
            
            // Join the space using room ID
            auto joinSpaceResponse = HttpClient::Post(
                m_sBaseUrl + "/_matrix/client/v3/rooms/" + spaceRoomId + "/join",
                "{}",
                headers
            );
            
            if (joinSpaceResponse.status_code == 200)
            {
                DEBUG_MSG("Successfully joined space: " + spaceRoomId);
                
                // Look for the talk room within the space hierarchy
                auto hierarchyResponse = HttpClient::Get(
                    m_sBaseUrl + "/_matrix/client/v1/rooms/" + spaceRoomId + "/hierarchy",
                    headers
                );
                
                DEBUG_MSG("Space hierarchy response: " + std::to_string(hierarchyResponse.status_code));
                
                if (hierarchyResponse.status_code == 200)
                {
                    auto hierarchyJson = nlohmann::json::parse(hierarchyResponse.text);
                    DEBUG_MSG("Got space hierarchy, looking for room: " + Vars::Chat::Room.Value);
                    DEBUG_MSG("Hierarchy response: " + hierarchyResponse.text);
                    
                    if (hierarchyJson.contains("rooms"))
                    {
                        DEBUG_MSG("Found " + std::to_string(hierarchyJson["rooms"].size()) + " rooms in hierarchy");
                        
                        for (const auto& room : hierarchyJson["rooms"])
                        {
                            std::string roomName = room.contains("name") ? room["name"].get<std::string>() : "unnamed";
                            std::string roomId = room.contains("room_id") ? room["room_id"].get<std::string>() : "unknown";
                            
                            DEBUG_MSG("Hierarchy room: " + roomName + " (" + roomId + ")");
                            
                            if (room.contains("name") && room["name"] == Vars::Chat::Room.Value)
                            {
                                std::string targetRoomId = room["room_id"];
                                DEBUG_MSG("Found target room in hierarchy: " + targetRoomId);
                                
                                // Join the specific room
                                auto roomJoinResponse = HttpClient::Post(
                                    m_sBaseUrl + "/_matrix/client/v3/rooms/" + targetRoomId + "/join",
                                    "{}",
                                    headers
                                );
                                
                                if (roomJoinResponse.status_code == 200)
                                {
                                    m_sRoomId = targetRoomId;
                                    DEBUG_MSG("Successfully joined target room: " + targetRoomId);
                                    return true;
                                }
                                else
                                {
                                    DEBUG_MSG("Failed to join target room: " + std::to_string(roomJoinResponse.status_code));
                                }
                            }
                        }
                    }
                    else
                    {
                        DEBUG_MSG("No rooms found in hierarchy");
                    }
                }
                else
                {
                    DEBUG_MSG("Failed to get space hierarchy: " + hierarchyResponse.text);
                    DEBUG_MSG("Trying to join room directly by alias...");
                    
                    // Try to join the room directly by alias since hierarchy failed
                    std::string roomAlias = "#" + Vars::Chat::Room.Value + ":" + Vars::Chat::Server.Value;
                    std::string encodedRoomAlias = HttpClient::UrlEncode(roomAlias);
                    std::string roomDirectoryUrl = m_sBaseUrl + "/_matrix/client/v3/directory/room/" + encodedRoomAlias;
                    
                    auto roomDirectoryResponse = HttpClient::Get(roomDirectoryUrl, headers);
                    
                    if (roomDirectoryResponse.status_code == 200)
                    {
                        auto roomDirectoryJson = nlohmann::json::parse(roomDirectoryResponse.text);
                        std::string targetRoomId = roomDirectoryJson["room_id"];
                        DEBUG_MSG("Found room " + roomAlias + " -> " + targetRoomId);
                        
                        // Try to join it
                        auto roomJoinResponse = HttpClient::Post(
                            m_sBaseUrl + "/_matrix/client/v3/rooms/" + targetRoomId + "/join",
                            "{}",
                            headers
                        );
                        
                        if (roomJoinResponse.status_code == 200)
                        {
                            m_sRoomId = targetRoomId;
                            DEBUG_MSG("Successfully joined room directly: " + targetRoomId);
                            return true;
                        }
                        else
                        {
                            DEBUG_MSG("Failed to join room " + roomAlias + ": " + std::to_string(roomJoinResponse.status_code));
                        }
                    }
                    else
                    {
                        DEBUG_MSG("Room " + roomAlias + " not found in directory");
                    }
                }
                
                // Use the space itself for messaging if no specific room found
                m_sRoomId = spaceRoomId;
                DEBUG_MSG("Using space as messaging room: " + spaceRoomId);
                return true;
            }
        }
        
        // Strategy 2: Try direct room lookup and join
        std::string roomAlias = "#" + Vars::Chat::Room.Value + ":" + Vars::Chat::Server.Value;
        std::string encodedRoomAlias = HttpClient::UrlEncode(roomAlias);
        std::string roomDirectoryUrl = m_sBaseUrl + "/_matrix/client/v3/directory/room/" + encodedRoomAlias;
        
        DEBUG_MSG("Looking up room directly: " + roomAlias);
        DEBUG_MSG("Encoded room alias: " + encodedRoomAlias);
        DEBUG_MSG("Room directory URL: " + roomDirectoryUrl);
        
        auto roomDirectoryResponse = HttpClient::Get(roomDirectoryUrl, headers);
        
        DEBUG_MSG("Room directory response code: " + std::to_string(roomDirectoryResponse.status_code));
        DEBUG_MSG("Room directory response: " + roomDirectoryResponse.text);
        
        if (roomDirectoryResponse.status_code == 200)
        {
            auto roomDirectoryJson = nlohmann::json::parse(roomDirectoryResponse.text);
            std::string targetRoomId = roomDirectoryJson["room_id"];
            DEBUG_MSG("Found room in directory: " + targetRoomId);
            
            // Join the room using room ID
            auto joinRoomResponse = HttpClient::Post(
                m_sBaseUrl + "/_matrix/client/v3/rooms/" + targetRoomId + "/join",
                "{}",
                headers
            );
            
            if (joinRoomResponse.status_code == 200)
            {
                m_sRoomId = targetRoomId;
                DEBUG_MSG("Successfully joined room directly: " + targetRoomId);
                return true;
            }
        }
        
        // Strategy 3: Try direct join using alias (bypass directory)
        DEBUG_MSG("Trying direct join with alias: " + spaceAlias);
        
        auto directJoinSpaceResponse = HttpClient::Post(
            m_sBaseUrl + "/_matrix/client/v3/join/" + HttpClient::UrlEncode(spaceAlias),
            "{}",
            headers
        );
        
        DEBUG_MSG("Direct space join response: " + std::to_string(directJoinSpaceResponse.status_code));
        DEBUG_MSG("Direct space join response body: " + directJoinSpaceResponse.text);
        
        if (directJoinSpaceResponse.status_code == 200)
        {
            auto spaceJson = nlohmann::json::parse(directJoinSpaceResponse.text);
            m_sRoomId = spaceJson["room_id"];
            DEBUG_MSG("Successfully joined space via direct join: " + m_sRoomId);
            return true;
        }
        
        DEBUG_MSG("Trying direct join with room alias: " + roomAlias);
        
        auto directJoinRoomResponse = HttpClient::Post(
            m_sBaseUrl + "/_matrix/client/v3/join/" + HttpClient::UrlEncode(roomAlias),
            "{}",
            headers
        );
        
        DEBUG_MSG("Direct room join response: " + std::to_string(directJoinRoomResponse.status_code));
        DEBUG_MSG("Direct room join response body: " + directJoinRoomResponse.text);
        
        if (directJoinRoomResponse.status_code == 200)
        {
            auto roomJson = nlohmann::json::parse(directJoinRoomResponse.text);
            m_sRoomId = roomJson["room_id"];
            DEBUG_MSG("Successfully joined room via direct join: " + m_sRoomId);
            return true;
        }
        
        // Strategy 3.5: Try alternative alias formats in case the server uses different conventions
        std::vector<std::string> alternativeAliases = {
            "#" + Vars::Chat::Space.Value + ":" + Vars::Chat::Server.Value,  // Original
            "!" + Vars::Chat::Space.Value + ":" + Vars::Chat::Server.Value,  // Room ID format
            "#" + Vars::Chat::Room.Value + ":" + Vars::Chat::Server.Value,   // Direct room
            "!" + Vars::Chat::Room.Value + ":" + Vars::Chat::Server.Value,   // Room ID format
        };
        
        for (const auto& alias : alternativeAliases)
        {
            DEBUG_MSG("Trying alternative alias: " + alias);
            
            auto altJoinResponse = HttpClient::Post(
                m_sBaseUrl + "/_matrix/client/v3/join/" + HttpClient::UrlEncode(alias),
                "{}",
                headers
            );
            
            if (altJoinResponse.status_code == 200)
            {
                auto altJson = nlohmann::json::parse(altJoinResponse.text);
                m_sRoomId = altJson["room_id"];
                DEBUG_MSG("Successfully joined with alternative alias: " + alias);
                return true;
            }
            else
            {
                DEBUG_MSG("Alternative alias failed (" + std::to_string(altJoinResponse.status_code) + "): " + altJoinResponse.text);
            }
        }
        
        // Strategy 4: Check what public rooms are available
        DEBUG_MSG("Checking public rooms directory...");
        
        auto publicRoomsResponse = HttpClient::Get(
            m_sBaseUrl + "/_matrix/client/v3/publicRooms?limit=10",
            headers
        );
        
        if (publicRoomsResponse.status_code == 200)
        {
            try {
                auto publicJson = nlohmann::json::parse(publicRoomsResponse.text);
                if (publicJson.contains("chunk"))
                {
                    DEBUG_MSG("Found " + std::to_string(publicJson["chunk"].size()) + " public rooms:");
                    for (const auto& room : publicJson["chunk"])
                    {
                        if (room.contains("name") && room.contains("room_id"))
                        {
                            std::string roomName = room["name"];
                            std::string roomId = room["room_id"];
                            DEBUG_MSG("- " + roomName + " (" + roomId + ")");
                            
                            // If we find any room that looks like it might be ours, try to join it
                            if (roomName.find("talk") != std::string::npos || 
                                roomName.find("async") != std::string::npos ||
                                roomName.find("amalgam") != std::string::npos)
                            {
                                DEBUG_MSG("Attempting to join promising room: " + roomName);
                                
                                auto joinPublicResponse = HttpClient::Post(
                                    m_sBaseUrl + "/_matrix/client/v3/rooms/" + roomId + "/join",
                                    "{}",
                                    headers
                                );
                                
                                if (joinPublicResponse.status_code == 200)
                                {
                                    m_sRoomId = roomId;
                                    DEBUG_MSG("Successfully joined public room: " + roomName);
                                    return true;
                                }
                            }
                        }
                    }
                }
            } catch (...) {
                DEBUG_MSG("Error parsing public rooms response");
            }
        }
        
        // Strategy 4.5: List all joined rooms and look for matches
        DEBUG_MSG("Checking existing joined rooms...");
        
        auto syncResponse = HttpClient::Get(
            m_sBaseUrl + "/_matrix/client/v3/sync?timeout=0",
            headers
        );
        
        if (syncResponse.status_code == 200)
        {
            auto syncJson = nlohmann::json::parse(syncResponse.text);
            
            if (syncJson.contains("rooms") && syncJson["rooms"].contains("join"))
            {
                DEBUG_MSG("Found " + std::to_string(syncJson["rooms"]["join"].size()) + " joined rooms");
                
                for (const auto& [roomId, roomData] : syncJson["rooms"]["join"].items())
                {
                    DEBUG_MSG("Checking room: " + roomId);
                    
                    // Simple check - just use the first room we find
                    if (syncJson["rooms"]["join"].size() > 0)
                    {
                        m_sRoomId = roomId;
                        DEBUG_MSG("Using existing room: " + roomId);
                        return true;
                    }
                }
            }
        }
        
        // Strategy 5: Create the room/space if they don't exist
        DEBUG_MSG("Creating asyncroom space since it doesn't exist...");
        
        nlohmann::json createSpaceData = {
            {"name", "Amalgam AsyncRoom"},
            {"room_alias_name", Vars::Chat::Space.Value},
            {"topic", "Amalgam TF2 Cheat Community Space"},
            {"preset", "public_chat"},
            {"visibility", "public"},
            {"creation_content", {{"type", "m.space"}}}
        };
        
        auto createSpaceResponse = HttpClient::Post(
            m_sBaseUrl + "/_matrix/client/v3/createRoom",
            createSpaceData.dump(),
            headers
        );
        
        if (createSpaceResponse.status_code == 200)
        {
            auto spaceJson = nlohmann::json::parse(createSpaceResponse.text);
            std::string spaceId = spaceJson["room_id"];
            
            DEBUG_MSG("Successfully created space: " + spaceId);
            
            // Now create the talk room within the space
            nlohmann::json createRoomData = {
                {"name", "Talk"},
                {"room_alias_name", Vars::Chat::Room.Value},
                {"topic", "General chat for Amalgam users"},
                {"preset", "public_chat"},
                {"visibility", "public"}
            };
            
            auto createRoomResponse = HttpClient::Post(
                m_sBaseUrl + "/_matrix/client/v3/createRoom",
                createRoomData.dump(),
                headers
            );
            
            if (createRoomResponse.status_code == 200)
            {
                auto roomJson = nlohmann::json::parse(createRoomResponse.text);
                m_sRoomId = roomJson["room_id"];
                
                DEBUG_MSG("Successfully created talk room: " + m_sRoomId);
                
                // Add room to space hierarchy
                nlohmann::json spaceChildData = {
                    {"via", {Vars::Chat::Server.Value}}
                };
                
                auto addToSpaceResponse = HttpClient::Put(
                    m_sBaseUrl + "/_matrix/client/v3/rooms/" + spaceId + "/state/m.space.child/" + m_sRoomId,
                    spaceChildData.dump(),
                    headers
                );
                
                DEBUG_MSG("Added room to space hierarchy");
                return true;
            }
            else
            {
                // Use the space itself for messaging
                m_sRoomId = spaceId;
                DEBUG_MSG("Using space for messaging: " + spaceId);
                return true;
            }
        }
        
        // Strategy 6: Just create a simple public room called "amalgam"
        DEBUG_MSG("Creating fallback room: amalgam");
        
        nlohmann::json fallbackRoomData = {
            {"name", "Amalgam Chat"},
            {"room_alias_name", "amalgam"},
            {"topic", "Amalgam TF2 Cheat Chat Room"},
            {"preset", "public_chat"},
            {"visibility", "public"}
        };
        
        auto fallbackResponse = HttpClient::Post(
            m_sBaseUrl + "/_matrix/client/v3/createRoom",
            fallbackRoomData.dump(),
            headers
        );
        
        if (fallbackResponse.status_code == 200)
        {
            auto roomJson = nlohmann::json::parse(fallbackResponse.text);
            m_sRoomId = roomJson["room_id"];
            DEBUG_MSG("Successfully created fallback room: " + m_sRoomId);
            return true;
        }
        
        DEBUG_MSG("All strategies failed including room creation");
        std::lock_guard<std::mutex> lock(m_StatusMutex);
        m_sLastError = "Could not find, join, or create any rooms. Space: " + spaceAlias + ", Room: " + roomAlias;
        return false;
    }
    catch (const std::exception& e)
    {
        std::lock_guard<std::mutex> lock(m_StatusMutex);
        m_sLastError = "Join room error: " + std::string(e.what());
        return false;
    }
}

bool CChat::HttpSendMessage(const std::string& message)
{
    try
    {
        DEBUG_MSG("Attempting to send message to room: " + m_sRoomId);
        
        std::string eventType = "m.room.message";
        std::string content;
        
        // Prepare message content
        nlohmann::json msgData = {
            {"msgtype", "m.text"},
            {"body", message}
        };
        
        // Check if encryption should be used for this room
        bool shouldEncrypt = ShouldEncryptMessage(m_sRoomId);
        bool isEncryptionEnabled = IsEncryptionEnabled();
        bool isRoomEncrypted = IsRoomEncrypted(m_sRoomId);
        
        SDK::Output("Matrix", "=== ENCRYPTION DECISION DEBUG ===", { 255, 255, 100, 255 });
        SDK::Output("Matrix", ("Room ID: " + m_sRoomId).c_str(), { 200, 200, 200, 255 });
        SDK::Output("Matrix", ("Crypto pointer: " + std::string(m_pCrypto ? "VALID" : "NULL")).c_str(), { 200, 200, 200, 255 });
        SDK::Output("Matrix", ("m_bEncryptionEnabled: " + std::string(m_bEncryptionEnabled ? "TRUE" : "FALSE")).c_str(), { 200, 200, 200, 255 });
        SDK::Output("Matrix", ("Global encryption enabled: " + std::string(isEncryptionEnabled ? "YES" : "NO")).c_str(), { 200, 200, 200, 255 });
        SDK::Output("Matrix", ("Room is encrypted: " + std::string(isRoomEncrypted ? "YES" : "NO")).c_str(), { 200, 200, 200, 255 });
        SDK::Output("Matrix", ("Should encrypt message: " + std::string(shouldEncrypt ? "YES" : "NO")).c_str(), { 200, 200, 200, 255 });
        
        // Show all room encryption states for debugging
        {
            std::lock_guard<std::mutex> lock(m_EncryptionStateMutex);
            SDK::Output("Matrix", ("Room encryption states (" + std::to_string(m_RoomEncryptionState.size()) + " total):").c_str(), { 200, 200, 200, 255 });
            for (const auto& [roomId, encrypted] : m_RoomEncryptionState) {
                SDK::Output("Matrix", ("  " + roomId + " -> " + std::string(encrypted ? "ENCRYPTED" : "NOT_ENCRYPTED")).c_str(), { 200, 200, 200, 255 });
            }
        }
        
        // TEMPORARY DEBUG: Force encryption initialization if encryption is not enabled
        // Since we know the room is encrypted but global encryption is disabled, force initialization
        SDK::Output("Matrix", ("DEBUG: Message content: '" + message + "'").c_str(), { 255, 100, 255, 255 });
        SDK::Output("Matrix", ("DEBUG: Message contains 'hi': " + std::string(message.find("hi") != std::string::npos ? "YES" : "NO")).c_str(), { 255, 100, 255, 255 });
        SDK::Output("Matrix", ("DEBUG: Encryption enabled: " + std::string(isEncryptionEnabled ? "YES" : "NO")).c_str(), { 255, 100, 255, 255 });
        
        // Force encryption initialization for any message when encryption is disabled but room is encrypted
        bool forceEncryptForTesting = !isEncryptionEnabled && isRoomEncrypted;
        
        if (forceEncryptForTesting) {
            SDK::Output("Matrix", "DEBUG: Encryption not initialized yet, checking why...", { 255, 255, 0, 255 });
            SDK::Output("Matrix", ("Connected status: " + std::string(m_bConnected.load() ? "YES" : "NO")).c_str(), { 200, 200, 200, 255 });
            SDK::Output("Matrix", ("Access token: " + std::string(m_sAccessToken.empty() ? "EMPTY" : "SET")).c_str(), { 200, 200, 200, 255 });
            SDK::Output("Matrix", ("Room ID: " + std::string(m_sRoomId.empty() ? "EMPTY" : "SET")).c_str(), { 200, 200, 200, 255 });
            
            SDK::Output("Matrix", "Forcing encryption initialization (message starts with !!)", { 255, 255, 0, 255 });
            // Try to initialize encryption now
            bool initResult = InitializeEncryption();
            SDK::Output("Matrix", ("Manual InitializeEncryption() result: " + std::string(initResult ? "SUCCESS" : "FAILED")).c_str(), 
                       initResult ? Color_t{150, 255, 150, 255} : Color_t{255, 150, 150, 255});
            
            if (initResult) {
                isEncryptionEnabled = IsEncryptionEnabled();
                shouldEncrypt = ShouldEncryptMessage(m_sRoomId);
                SDK::Output("Matrix", ("After manual init - Global encryption: " + std::string(isEncryptionEnabled ? "YES" : "NO")).c_str(), { 200, 255, 200, 255 });
                SDK::Output("Matrix", ("After manual init - Should encrypt: " + std::string(shouldEncrypt ? "YES" : "NO")).c_str(), { 200, 255, 200, 255 });
            }
        }
        
        if (shouldEncrypt) {
            SDK::Output("Matrix", "Encrypting message with Megolm", { 150, 255, 150, 255 });
            
            // Ensure room session exists first
            if (!m_pCrypto->EnsureRoomSession(m_sRoomId)) {
                SDK::Output("Matrix", "Failed to create/ensure room session", { 255, 150, 150, 255 });
                content = msgData.dump(); // Fall back to unencrypted
            } else {
                // Share session key with other users in the room
                std::vector<std::string> user_ids = m_vRoomMembers;
            
            // If we don't have room members yet, refresh them
            if (user_ids.empty()) {
                SDK::Output("Matrix", "No room members cached, refreshing...", { 255, 255, 150, 255 });
                HttpGetRoomMembers();
                user_ids = m_vRoomMembers;
                SDK::Output("Matrix", ("Found " + std::to_string(user_ids.size()) + " room members").c_str(), { 200, 255, 200, 255 });
            } else {
                SDK::Output("Matrix", ("Using cached " + std::to_string(user_ids.size()) + " room members").c_str(), { 200, 255, 200, 255 });
            }
            
            // Ensure we include ourselves if not already in the list
            std::string ourUserId = "@" + Vars::Chat::Username.Value + ":" + Vars::Chat::Server.Value;
            if (std::find(user_ids.begin(), user_ids.end(), ourUserId) == user_ids.end()) {
                user_ids.push_back(ourUserId);
            }
            
            // Ensure we have fresh device keys for all users
            SDK::Output("Matrix", ("Downloading device keys for " + std::to_string(user_ids.size()) + " users").c_str(), { 200, 255, 200, 255 });
            for (const auto& user_id : user_ids) {
                SDK::Output("Matrix", ("  User: " + user_id).c_str(), { 200, 200, 200, 255 });
            }
            bool deviceKeysResult = HttpDownloadDeviceKeys();
            SDK::Output("Matrix", ("Device keys download result: " + std::string(deviceKeysResult ? "SUCCESS" : "FAILED")).c_str(), 
                       deviceKeysResult ? Color_t{150, 255, 150, 255} : Color_t{255, 150, 150, 255});
            
            // Claim one-time keys for proper Olm session establishment
            SDK::Output("Matrix", "Claiming one-time keys for Olm sessions", { 200, 255, 200, 255 });
            bool claimKeysResult = HttpClaimKeys(user_ids);
            SDK::Output("Matrix", ("One-time keys claim result: " + std::string(claimKeysResult ? "SUCCESS" : "FAILED")).c_str(), 
                       claimKeysResult ? Color_t{150, 255, 150, 255} : Color_t{255, 150, 150, 255});
            
            // Share the session key with the users
            SDK::Output("Matrix", "Sharing session key with room members", { 200, 255, 200, 255 });
            bool shareResult = m_pCrypto->ShareSessionKey(m_sRoomId, user_ids);
            SDK::Output("Matrix", ("Session key sharing result: " + std::string(shareResult ? "SUCCESS" : "FAILED")).c_str(), 
                       shareResult ? Color_t{150, 255, 150, 255} : Color_t{255, 150, 150, 255});
            
            // Note: Skipping session key tests to prevent advancing the session index
            // The tests would interfere with the actual message encryption by advancing the session
            SDK::Output("Matrix", "Session key tests skipped to preserve session state for message encryption", { 255, 255, 100, 255 });
            
            // Send any pending room keys via to-device messages
            auto pendingKeys = m_pCrypto->GetAndClearPendingRoomKeys();
            SDK::Output("Matrix", ("Sending " + std::to_string(pendingKeys.size()) + " room key to-device messages").c_str(), { 255, 255, 150, 255 });
            
            // UTD FIX: Sort by sequence number and add delay between sends to prevent out-of-order processing
            std::sort(pendingKeys.begin(), pendingKeys.end(), [](const PendingRoomKey& a, const PendingRoomKey& b) {
                if (a.sequence_number != b.sequence_number) {
                    return a.sequence_number < b.sequence_number;
                }
                // Secondary sort by timestamp for stability
                return a.timestamp < b.timestamp;
            });
            
            // Track failed sends for potential retry
            std::vector<std::string> failedDevices;
            int successCount = 0;
            
            for (size_t i = 0; i < pendingKeys.size(); ++i) {
                const auto& key = pendingKeys[i];
                SDK::Output("Matrix", ("Sending to-device message to " + key.user_id + ":" + key.device_id + " (seq: " + std::to_string(key.sequence_number) + ")").c_str(), { 200, 200, 255, 255 });
                
                if (HttpSendToDevice("m.room.encrypted", key.user_id, key.device_id, key.encrypted_content)) {
                    successCount++;
                    SDK::Output("Matrix", ("To-device send SUCCESS for " + key.device_id + " (seq: " + std::to_string(key.sequence_number) + ")").c_str(), { 150, 255, 150, 255 });
                } else {
                    failedDevices.push_back(key.device_id);
                    SDK::Output("Matrix", ("To-device send FAILED for " + key.device_id + " (seq: " + std::to_string(key.sequence_number) + ")").c_str(), { 255, 150, 150, 255 });
                }
                
                // UTD FIX: Small delay between sends to prevent out-of-order message processing
                if (i < pendingKeys.size() - 1) { // Don't delay after the last message
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            }
            
            // UTD FIX: Improved retry logic for failed to-device messages
            if (failedDevices.empty()) {
                // No need to clear - GetAndClearPendingRoomKeys already cleared them
                DEBUG_MSG("All " + std::to_string(successCount) + " room key messages sent successfully");
            } else {
                DEBUG_MSG("Room key delivery: " + std::to_string(successCount) + " succeeded, " + 
                         std::to_string(failedDevices.size()) + " failed");
                
                // UTD FIX: Retry failed sends immediately (up to 3 attempts)
                int retryAttempts = 0;
                const int maxRetries = 3;
                
                while (retryAttempts < maxRetries && !failedDevices.empty()) {
                    retryAttempts++;
                    SDK::Output("Matrix", ("Retry attempt " + std::to_string(retryAttempts) + " for " + std::to_string(failedDevices.size()) + " failed devices").c_str(), { 255, 200, 100, 255 });
                    
                    std::vector<std::string> stillFailed;
                    
                    // Retry each failed device
                    for (const auto& failedDeviceId : failedDevices) {
                        // Find the pending key for this device
                        for (const auto& key : pendingKeys) {
                            if (key.device_id == failedDeviceId) {
                                SDK::Output("Matrix", ("Retrying to-device message to " + key.user_id + ":" + key.device_id + " (seq: " + std::to_string(key.sequence_number) + ")").c_str(), { 200, 200, 255, 255 });
                                
                                if (HttpSendToDevice("m.room.encrypted", key.user_id, key.device_id, key.encrypted_content)) {
                                    successCount++;
                                    SDK::Output("Matrix", ("Retry SUCCESS for " + key.device_id + " (seq: " + std::to_string(key.sequence_number) + ")").c_str(), { 150, 255, 150, 255 });
                                } else {
                                    stillFailed.push_back(failedDeviceId);
                                    SDK::Output("Matrix", ("Retry FAILED for " + key.device_id + " (seq: " + std::to_string(key.sequence_number) + ")").c_str(), { 255, 150, 150, 255 });
                                }
                                break;
                            }
                        }
                    }
                    
                    failedDevices = stillFailed;
                    
                    // Brief delay between retry attempts
                    if (!failedDevices.empty() && retryAttempts < maxRetries) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(200));
                    }
                }
                
                // Keys already cleared by GetAndClearPendingRoomKeys, no need to clear again
                
                if (!failedDevices.empty()) {
                    DEBUG_MSG("Final result: " + std::to_string(successCount) + " succeeded, " + 
                             std::to_string(failedDevices.size()) + " permanently failed after " + std::to_string(retryAttempts) + " retry attempts");
                    for (const auto& device : failedDevices) {
                        DEBUG_MSG("Permanently failed device: " + device);
                    }
                } else {
                    DEBUG_MSG("All room key messages eventually sent successfully after " + std::to_string(retryAttempts) + " retry attempts");
                }
            }
            
            // Brief delay to allow to-device messages to be processed
            if (!pendingKeys.empty()) {
                DEBUG_MSG("Waiting for to-device messages to be processed...");
                std::this_thread::sleep_for(std::chrono::milliseconds(2000)); // Increased delay for Element Web
            }
            
            // Encrypt the message content
            std::string encryptedContent = m_pCrypto->EncryptMessage(m_sRoomId, eventType, msgData.dump());
            if (!encryptedContent.empty()) {
                eventType = "m.room.encrypted";
                content = encryptedContent;
                SDK::Output("Matrix", "Message encrypted successfully", { 150, 255, 150, 255 });
                SDK::Output("Matrix", ("ENCRYPTED CONTENT: " + encryptedContent.substr(0, 200) + "...").c_str(), { 200, 200, 255, 255 });
                } else {
                    SDK::Output("Matrix", "Encryption failed, sending unencrypted", { 255, 150, 150, 255 });
                    content = msgData.dump();
                }
            }
        } else {
            // Encryption disabled, send unencrypted message
            content = msgData.dump();
            SDK::Output("Matrix", "Sending unencrypted message", { 255, 255, 150, 255 });
        }
        
        std::string txnId = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
        
        std::map<std::string, std::string> headers = {
            {"Authorization", "Bearer " + m_sAccessToken},
            {"Content-Type", "application/json"}
        };
        
        std::string url = m_sBaseUrl + "/_matrix/client/v3/rooms/" + m_sRoomId + "/send/" + eventType + "/" + txnId;
        DEBUG_MSG("Send URL: " + url);
        
        auto response = HttpClient::Put(url, content, headers);
        
        if (response.status_code == 200)
        {
            SDK::Output("Matrix", "Message sent successfully!", { 150, 255, 150, 255 });
            
            // Send any additional room keys that might have been generated
            SendPendingRoomKeys();
            
            // Don't show local echo - we'll receive it back from the server with proper formatting
            
            return true;
        }
        else
        {
            SDK::Output("Matrix", ("Send failed with status: " + std::to_string(response.status_code)).c_str(), { 255, 150, 150, 255 });
            std::lock_guard<std::mutex> lock(m_StatusMutex);
            m_sLastError = "Send message failed with status: " + std::to_string(response.status_code);
            return false;
        }
    }
    catch (const std::exception& e)
    {
        std::lock_guard<std::mutex> lock(m_StatusMutex);
        m_sLastError = "Send message error: " + std::string(e.what());
        DEBUG_MSG("Send exception: " + std::string(e.what()));
        return false;
    }
}

void CChat::HttpSync()
{
    EnsureInitialized();
    
    try
    {
        // Use shorter timeout for more responsive behavior
        std::string url = m_sBaseUrl + "/_matrix/client/v3/sync?timeout=10000";
        if (!m_sNextBatch.empty())
            url += "&since=" + m_sNextBatch;
        
        std::map<std::string, std::string> headers = {
            {"Authorization", "Bearer " + m_sAccessToken}
        };
        
        auto response = HttpClient::Get(url, headers);
        
        if (response.status_code == 200)
        {
            auto json = nlohmann::json::parse(response.text);
            
            // Debug: Show sync status
            static bool firstSync = true;
            if (firstSync)
            {
                DEBUG_MSG("First sync successful, monitoring for messages...");
                firstSync = false;
            }
            
            if (json.contains("next_batch"))
                m_sNextBatch = json["next_batch"];
            
            // Monitor and replenish one-time keys
            if (json.contains("device_one_time_keys_count"))
            {
                auto& keyCounts = json["device_one_time_keys_count"];
                int currentKeyCount = 0;
                
                if (keyCounts.contains("signed_curve25519"))
                {
                    currentKeyCount = keyCounts["signed_curve25519"];
                }
                
                SDK::Output("Matrix", ("One-time keys remaining: " + std::to_string(currentKeyCount)).c_str(), { 200, 200, 255, 255 });
                
                // Element-web compatible key thresholds (from Element's patterns)
                if (m_pCrypto && m_pCrypto->GetOlmAccount())
                {
                    const int MIN_OTK_COUNT = 5;     // Element-web minimum threshold
                    const int MAX_OTK_COUNT = 100;   // Element-web maximum upload
                    
                    if (currentKeyCount < MIN_OTK_COUNT)
                    {
                        SDK::Output("Matrix", ("Key count (" + std::to_string(currentKeyCount) + ") below Element-web threshold (" + std::to_string(MIN_OTK_COUNT) + "), replenishing...").c_str(), { 255, 255, 100, 255 });
                        
                        // Upload new one-time keys
                        if (HttpUploadKeys())
                        {
                            SDK::Output("Matrix", "One-time keys replenished successfully", { 150, 255, 150, 255 });
                        }
                        else
                        {
                            SDK::Output("Matrix", "Failed to replenish one-time keys", { 255, 150, 150, 255 });
                            
                            // Try key exhaustion recovery as fallback
                            if (m_pCrypto && m_pCrypto->HandleKeyExhaustion()) {
                                SDK::Output("Matrix", "Attempting key exhaustion recovery", { 255, 255, 100, 255 });
                            }
                        }
                    }
                }
            }
            
            // Debug: Show all rooms we're joined to
            if (json.contains("rooms") && json["rooms"].contains("join"))
            {
                static bool shownRooms = false;
                if (!shownRooms)
                {
                    DEBUG_MSG("Rooms we're joined to:");
                    for (const auto& [roomId, roomData] : json["rooms"]["join"].items())
                    {
                        DEBUG_MSG("- Room ID: " + roomId);
                    }
                    DEBUG_MSG("Looking for messages in: " + m_sRoomId);
                    shownRooms = true;
                }
            }
            
            // Process events from ALL joined rooms (like original working version)
            if (json.contains("rooms") && json["rooms"].contains("join"))
            {
                for (const auto& [roomId, roomData] : json["rooms"]["join"].items())
                {
                    // Process messages from all joined rooms, not just target room
                    if (roomData.contains("timeline") && roomData["timeline"].contains("events"))
                    {
                        for (const auto& event : roomData["timeline"]["events"])
                        {
                            std::string sender = event.contains("sender") ? event["sender"] : "Unknown";
                            std::string content;
                            bool processed = false;
                            
                            // Handle encrypted messages
                            if (event.contains("type") && event["type"] == "m.room.encrypted" && IsEncryptionEnabled())
                            {
                                DEBUG_MSG("Encrypted message received from " + sender);
                                DEBUG_MSG("Encrypted content: " + event["content"].dump());
                                if (event.contains("content")) {
                                    content = m_pCrypto->DecryptMessage(roomId, event["content"].dump());
                                    if (!content.empty() && !content.starts_with("[")) {
                                        DEBUG_MSG("Successfully decrypted message from " + sender + ": " + content);
                                        processed = true;
                                    } else {
                                        DEBUG_MSG("Decryption failed for message from " + sender + ": " + content);
                                        // Try to extract error information for debugging
                                        if (event["content"].contains("algorithm")) {
                                            DEBUG_MSG("Message algorithm: " + std::string(event["content"]["algorithm"]));
                                        }
                                        if (event["content"].contains("session_id")) {
                                            DEBUG_MSG("Message session_id: " + std::string(event["content"]["session_id"]));
                                        }
                                    }
                                } else {
                                    DEBUG_MSG("Encrypted event missing content");
                                }
                            }
                            // Handle room encryption state changes
                            else if (event.contains("type") && event["type"] == "m.room.encryption" && 
                                     event.contains("state_key") && event["state_key"] == "")
                            {
                                if (event.contains("content") && event["content"].contains("algorithm") &&
                                    event["content"]["algorithm"] == "m.megolm.v1.aes-sha2")
                                {
                                    DEBUG_MSG("Room encryption enabled via timeline state event");
                                    DEBUG_MSG("Room ID from timeline: " + roomId);
                                    DEBUG_MSG("Current tracked room ID: " + m_sRoomId);
                                    SetRoomEncryption(roomId, true);
                                }
                                processed = true; // Don't display state events as messages
                            }
                            // Handle unencrypted text messages
                            else if (event.contains("type") && event["type"] == "m.room.message" && 
                                     event.contains("content") && event["content"].contains("msgtype") && 
                                     event["content"]["msgtype"] == "m.text")
                            {
                                content = event["content"]["body"];
                                // Debug: Show message received with room info (like original working version)
                                DEBUG_MSG("Message in " + roomId + " from " + sender + ": " + content);
                                processed = true;
                            }
                            
                            if (processed && !content.empty()) {
                                ProcessIncomingMessage(sender, content);
                            }
                        }
                    }
                    
                    // Process initial room state events
                    if (roomId == m_sRoomId && roomData.contains("state") && roomData["state"].contains("events"))
                    {
                        for (const auto& event : roomData["state"]["events"])
                        {
                            // Check for room encryption state
                            if (event.contains("type") && event["type"] == "m.room.encryption" && 
                                event.contains("state_key") && event["state_key"] == "")
                            {
                                if (event.contains("content") && event["content"].contains("algorithm") &&
                                    event["content"]["algorithm"] == "m.megolm.v1.aes-sha2")
                                {
                                    DEBUG_MSG("Room encryption detected in initial state");
                                    DEBUG_MSG("Room ID from initial state: " + roomId);
                                    DEBUG_MSG("Current tracked room ID: " + m_sRoomId);
                                    SetRoomEncryption(roomId, true);
                                }
                            }
                        }
                    }
                }
            }
            
            // Process to-device events for encryption
            if (json.contains("to_device") && json["to_device"].contains("events") && m_pCrypto)
            {
                for (const auto& event : json["to_device"]["events"])
                {
                    if (event.contains("type") && event["type"] == "m.room_key")
                    {
                        // Process room key events
                        m_pCrypto->ProcessKeyShareEvent(event.dump());
                        DEBUG_MSG("Processed room key event");
                    }
                    else if (event.contains("type") && event["type"] == "m.room.encrypted")
                    {
                        // Decrypt and process encrypted to-device messages
                        // This would contain room keys encrypted with Olm
                        DEBUG_MSG("Received encrypted to-device message");
                        if (event.contains("content")) {
                            std::string decrypted_content = m_pCrypto->DecryptToDeviceMessage(event["content"].dump());
                            if (!decrypted_content.empty() && !decrypted_content.starts_with("[")) {
                                DEBUG_MSG("Decrypted to-device message: " + decrypted_content);
                                // Process the decrypted content (should be a room key event)
                                m_pCrypto->ProcessKeyShareEvent(decrypted_content);
                                DEBUG_MSG("Processed decrypted room key event");
                            } else {
                                DEBUG_MSG("To-device decryption failed: " + decrypted_content);
                            }
                        }
                    }
                }
            }
        }
        else if (response.status_code == 0)
        {
            // Network timeout or connection issue - this is normal for long polling
            // Don't log anything to avoid spam
        }
        else
        {
            // Log unexpected HTTP errors (but don't spam)
            static int errorCount = 0;
            if (errorCount < 3)
            {
                DEBUG_MSG("Sync HTTP error " + std::to_string(response.status_code) + ": " + response.text);
                errorCount++;
            }
        }
    }
    catch (const std::exception& e)
    {
        (void)e;
        // Log sync errors for debugging (but don't spam)
        static int errorCount = 0;
        if (errorCount < 3)
        {
            DEBUG_MSG("Sync exception: " + std::string(e.what()));
            errorCount++;
        }
    }
}

bool CChat::HttpUploadDeviceKeys()
{
    EnsureInitialized();
    
    if (!m_pCrypto) {
        DEBUG_MSG("Cannot upload device keys - encryption not initialized");
        return false;
    }
    
    try {
        std::string deviceKeysJson = m_pCrypto->GetDeviceKeys();
        if (deviceKeysJson.empty()) {
            DEBUG_MSG("Failed to get device keys");
            return false;
        }
        
        // Parse device keys and format exactly like Element-web's IUploadKeysRequest
        auto deviceKeys = nlohmann::json::parse(deviceKeysJson);
        
        nlohmann::json uploadData = {
            {"device_keys", deviceKeys}
        };
        
        std::map<std::string, std::string> headers = {
            {"Authorization", "Bearer " + m_sAccessToken},
            {"Content-Type", "application/json"}
        };
        
        std::string url = m_sBaseUrl + "/_matrix/client/v3/keys/upload";
        auto response = HttpClient::Post(url, uploadData.dump(), headers);
        
        if (response.status_code == 200) {
            DEBUG_MSG("Device keys uploaded successfully");
            return true;
        } else {
            // Provide detailed error information to console
            std::string errorMsg = "Failed to upload device keys (HTTP " + std::to_string(response.status_code) + ")";
            if (response.status_code == 401) {
                errorMsg += " - Authentication failed";
            } else if (response.status_code == 403) {
                errorMsg += " - Forbidden (check permissions)";
            } else if (response.status_code == 429) {
                errorMsg += " - Rate limited";
            } else if (response.status_code >= 500) {
                errorMsg += " - Server error (may be temporary)";
            }
            if (!response.text.empty()) {
                errorMsg += " - " + response.text;
            }
            SDK::Output("Matrix", errorMsg.c_str(), { 255, 150, 150, 255 });
            DEBUG_MSG("Failed to upload device keys: " + std::to_string(response.status_code) + " - " + response.text);
            return false;
        }
    } catch (const std::exception& e) {
        (void)e;
        std::string errorMsg = "Device key upload error: " + std::string(e.what());
        SDK::Output("Matrix", errorMsg.c_str(), { 255, 150, 150, 255 });
        DEBUG_MSG("Device key upload error: " + std::string(e.what()));
        return false;
    }
}

bool CChat::HttpDownloadDeviceKeys(const std::string& user_id)
{
    if (!m_pCrypto) {
        return false;
    }
    
    try {
        nlohmann::json queryData = {
            {"device_keys", {}}
        };
        
        // If user_id is specified, query just that user, otherwise query all users in the room
        if (!user_id.empty()) {
            queryData["device_keys"][user_id] = nlohmann::json::array();
        } else {
            // Query device keys for all room members
            if (!m_vRoomMembers.empty()) {
                for (const auto& member : m_vRoomMembers) {
                    queryData["device_keys"][member] = nlohmann::json::array();
                }
                DEBUG_MSG("Querying device keys for " + std::to_string(m_vRoomMembers.size()) + " room members");
            } else {
                // Fallback: query our own user (construct from username and server)
                std::string our_user_id = "@" + Vars::Chat::Username.Value + ":" + Vars::Chat::Server.Value;
                queryData["device_keys"][our_user_id] = nlohmann::json::array();
                DEBUG_MSG("No room members cached, querying own device keys only");
            }
        }
        
        std::map<std::string, std::string> headers = {
            {"Authorization", "Bearer " + m_sAccessToken},
            {"Content-Type", "application/json"}
        };
        
        std::string url = m_sBaseUrl + "/_matrix/client/v3/keys/query";
        auto response = HttpClient::Post(url, queryData.dump(), headers);
        
        if (response.status_code == 200) {
            auto responseJson = nlohmann::json::parse(response.text);
            if (responseJson.contains("device_keys")) {
                // Process the device keys
                for (auto& [userId, devices] : responseJson["device_keys"].items()) {
                    std::map<std::string, MatrixDevice> userDevices;
                    for (auto& [deviceId, deviceInfo] : devices.items()) {
                        MatrixDevice device;
                        device.device_id = deviceId;
                        device.user_id = userId;
                        
                        if (deviceInfo.contains("keys")) {
                            for (auto& [keyId, keyValue] : deviceInfo["keys"].items()) {
                                if (keyId.find("curve25519:") == 0) {
                                    device.curve25519_key = keyValue;
                                } else if (keyId.find("ed25519:") == 0) {
                                    device.ed25519_key = keyValue;
                                }
                            }
                        }
                        
                        userDevices[deviceId] = device;
                    }
                    
                    m_pCrypto->UpdateUserDevices(userId, userDevices);
                }
                DEBUG_MSG("Downloaded device keys for " + std::to_string(responseJson["device_keys"].size()) + " users");
                return true;
            }
        } else {
            DEBUG_MSG("Failed to download device keys: " + std::to_string(response.status_code));
        }
        
        return false;
    } catch (const std::exception& e) {
        (void)e;
        DEBUG_MSG("Device key download error: " + std::string(e.what()));
        return false;
    }
}

bool CChat::HttpSendToDevice(const std::string& event_type, const std::string& target_user, const std::string& target_device, const std::string& content)
{
    try {
        // Debug: Log what we're trying to send
        SDK::Output("Matrix", ("TO-DEVICE ATTEMPT: " + event_type + " to " + target_user + ":" + target_device).c_str(), { 200, 200, 255, 255 });
        SDK::Output("Matrix", ("TO-DEVICE CONTENT: '" + content.substr(0, 100) + "...'").c_str(), { 200, 200, 255, 255 });
        
        nlohmann::json messages = {
            {target_user, {
                {target_device, nlohmann::json::parse(content)}
            }}
        };
        
        nlohmann::json sendData = {
            {"messages", messages}
        };
        
        std::map<std::string, std::string> headers = {
            {"Authorization", "Bearer " + m_sAccessToken},
            {"Content-Type", "application/json"}
        };
        
        // Generate transaction ID with timestamp for better uniqueness
        static int txnCounter = 0;
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        std::string txnId = "txn" + std::to_string(timestamp) + "_" + std::to_string(++txnCounter);
        
        std::string url = m_sBaseUrl + "/_matrix/client/v3/sendToDevice/" + event_type + "/" + txnId;
        
        // Debug: Log the to-device message being sent
        SDK::Output("Matrix", ("TO-DEVICE URL: " + url).c_str(), { 200, 200, 255, 255 });
        SDK::Output("Matrix", ("TO-DEVICE PAYLOAD: " + sendData.dump().substr(0, 300) + "...").c_str(), { 200, 200, 255, 255 });
        
        auto response = HttpClient::Put(url, sendData.dump(), headers);
        
        // Debug: Log the response
        SDK::Output("Matrix", ("TO-DEVICE RESPONSE STATUS: " + std::to_string(response.status_code)).c_str(), { 200, 200, 255, 255 });
        
        if (response.status_code == 200) {
            SDK::Output("Matrix", ("To-device message sent: " + event_type + " to " + target_device).c_str(), { 150, 255, 150, 255 });
            SDK::Output("Matrix", ("TO-DEVICE RESPONSE: " + response.text).c_str(), { 150, 255, 150, 255 });
            return true;
        } else {
            // Provide more detailed error information
            std::string errorMsg = "Failed to send to-device message (HTTP " + std::to_string(response.status_code) + ")";
            if (response.status_code == 401) {
                errorMsg += " - Authentication failed";
            } else if (response.status_code == 403) {
                errorMsg += " - Forbidden (check permissions)";
            } else if (response.status_code == 429) {
                errorMsg += " - Rate limited";
            } else if (response.status_code >= 500) {
                errorMsg += " - Server error (may be temporary)";
            }
            if (!response.text.empty()) {
                errorMsg += " - " + response.text;
            }
            // Force detailed error to console
            SDK::Output("Matrix", ("TO-DEVICE ERROR: " + errorMsg).c_str(), { 255, 150, 150, 255 });
            DEBUG_MSG(errorMsg);
            return false;
        }
    } catch (const std::exception& e) {
        (void)e;
        std::string errorMsg = "To-device send exception: " + std::string(e.what());
        SDK::Output("Matrix", ("TO-DEVICE EXCEPTION: " + errorMsg).c_str(), { 255, 150, 150, 255 });
        DEBUG_MSG("To-device send error: " + std::string(e.what()));
        return false;
    }
}

bool CChat::HttpUploadOneTimeKeys()
{
    EnsureInitialized();
    
    if (!m_pCrypto) {
        return false;
    }
    
    try {
        std::string oneTimeKeysJson = m_pCrypto->GetOneTimeKeys();
        if (oneTimeKeysJson.empty()) {
            DEBUG_MSG("No one-time keys to upload");
            return true;
        }
        
        nlohmann::json uploadData = {
            {"one_time_keys", nlohmann::json::parse(oneTimeKeysJson)}
        };
        
        std::map<std::string, std::string> headers = {
            {"Authorization", "Bearer " + m_sAccessToken},
            {"Content-Type", "application/json"}
        };
        
        std::string url = m_sBaseUrl + "/_matrix/client/v3/keys/upload";
        auto response = HttpClient::Post(url, uploadData.dump(), headers);
        
        if (response.status_code == 200) {
            m_pCrypto->MarkOneTimeKeysAsPublished();
            DEBUG_MSG("One-time keys uploaded successfully");
            return true;
        } else {
            // Provide detailed error information to console
            std::string errorMsg = "Failed to upload one-time keys (HTTP " + std::to_string(response.status_code) + ")";
            if (response.status_code == 401) {
                errorMsg += " - Authentication failed";
            } else if (response.status_code == 403) {
                errorMsg += " - Forbidden (check permissions)";
            } else if (response.status_code == 429) {
                errorMsg += " - Rate limited";
            } else if (response.status_code >= 500) {
                errorMsg += " - Server error (may be temporary)";
            }
            if (!response.text.empty()) {
                errorMsg += " - " + response.text;
            }
            SDK::Output("Matrix", errorMsg.c_str(), { 255, 150, 150, 255 });
            DEBUG_MSG("Failed to upload one-time keys: " + std::to_string(response.status_code));
            return false;
        }
    } catch (const std::exception& e) {
        (void)e;
        std::string errorMsg = "One-time key upload error: " + std::string(e.what());
        SDK::Output("Matrix", errorMsg.c_str(), { 255, 150, 150, 255 });
        DEBUG_MSG("One-time key upload error: " + std::string(e.what()));
        return false;
    }
}

bool CChat::HttpUploadKeys()
{
    EnsureInitialized();
    
    if (!m_pCrypto) {
        return false;
    }
    
    try {
        // Generate new one-time keys to replenish the server
        SDK::Output("Matrix", "Generating new one-time keys for replenishment", { 200, 200, 255, 255 });
        
        // Get max keys and calculate how many to generate
        size_t maxKeys = olm_account_max_number_of_one_time_keys(m_pCrypto->GetOlmAccount());
        size_t keysToGenerate = maxKeys / 2; // Generate enough to reach target
        
        SDK::Output("Matrix", ("Generating " + std::to_string(keysToGenerate) + " new one-time keys (max: " + std::to_string(maxKeys) + ")").c_str(), { 200, 200, 255, 255 });
        
        // Generate random data for key generation
        size_t randomLength = olm_account_generate_one_time_keys_random_length(m_pCrypto->GetOlmAccount(), keysToGenerate);
        std::vector<uint8_t> randomBuffer(randomLength);
        
        // Fill with random data
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<unsigned int> dis(0, 255);
        for (size_t i = 0; i < randomLength; ++i) {
            randomBuffer[i] = static_cast<uint8_t>(dis(gen));
        }
        
        // Generate the keys
        size_t result = olm_account_generate_one_time_keys(m_pCrypto->GetOlmAccount(), keysToGenerate, randomBuffer.data(), randomLength);
        if (result == olm_error()) {
            SDK::Output("Matrix", "Failed to generate one-time keys", { 255, 150, 150, 255 });
            return false;
        }
        
        // Get the new keys
        std::string oneTimeKeysJson = m_pCrypto->GetOneTimeKeys();
        if (oneTimeKeysJson.empty()) {
            SDK::Output("Matrix", "No new one-time keys generated", { 255, 200, 100, 255 });
            return true;
        }
        
        // Format the upload data to match Element-web's IUploadKeysRequest interface
        // Only upload one-time keys during replenishment (device keys already uploaded during initial setup)
        auto oneTimeKeys = nlohmann::json::parse(oneTimeKeysJson);
        
        nlohmann::json uploadData = {
            {"one_time_keys", oneTimeKeys}
        };
        
        std::map<std::string, std::string> headers = {
            {"Authorization", "Bearer " + m_sAccessToken},
            {"Content-Type", "application/json"}
        };
        
        std::string url = m_sBaseUrl + "/_matrix/client/v3/keys/upload";
        auto response = HttpClient::Post(url, uploadData.dump(), headers);
        
        if (response.status_code == 200) {
            m_pCrypto->MarkOneTimeKeysAsPublished();
            
            // Parse response to get updated key counts
            auto responseJson = nlohmann::json::parse(response.text);
            if (responseJson.contains("one_time_key_counts")) {
                auto& keyCounts = responseJson["one_time_key_counts"];
                if (keyCounts.contains("signed_curve25519")) {
                    int newKeyCount = keyCounts["signed_curve25519"];
                    SDK::Output("Matrix", ("Keys uploaded successfully! New server count: " + std::to_string(newKeyCount)).c_str(), { 150, 255, 150, 255 });
                }
            }
            
            return true;
        } else {
            // Provide detailed error information to console
            std::string errorMsg = "Failed to upload keys (HTTP " + std::to_string(response.status_code) + ")";
            if (response.status_code == 401) {
                errorMsg += " - Authentication failed";
            } else if (response.status_code == 403) {
                errorMsg += " - Forbidden (check permissions)";
            } else if (response.status_code == 429) {
                errorMsg += " - Rate limited";
            } else if (response.status_code >= 500) {
                errorMsg += " - Server error (may be temporary)";
                
                // For server errors, implement exponential backoff and retry
                static int retry_count = 0;
                static auto last_failure = std::chrono::steady_clock::now();
                auto now = std::chrono::steady_clock::now();
                auto time_since_failure = std::chrono::duration_cast<std::chrono::minutes>(now - last_failure).count();
                
                if (time_since_failure > 5) { // Reset retry count after 5 minutes
                    retry_count = 0;
                }
                
                if (retry_count < 3) {
                    errorMsg += " - Will retry in " + std::to_string((retry_count + 1) * 30) + " seconds";
                    retry_count++;
                    last_failure = now;
                } else {
                    errorMsg += " - Max retries exceeded, will try again later";
                }
            }
            if (!response.text.empty()) {
                errorMsg += " - " + response.text;
            }
            SDK::Output("Matrix", errorMsg.c_str(), { 255, 150, 150, 255 });
            return false;
        }
    } catch (const std::exception& e) {
        (void)e;
        SDK::Output("Matrix", ("Key upload error: " + std::string(e.what())).c_str(), { 255, 150, 150, 255 });
        return false;
    }
}

bool CChat::HttpGetRoomMembers()
{
    try {
        std::map<std::string, std::string> headers = {
            {"Authorization", "Bearer " + m_sAccessToken}
        };
        
        std::string url = m_sBaseUrl + "/_matrix/client/v3/rooms/" + m_sRoomId + "/members";
        auto response = HttpClient::Get(url, headers);
        
        if (response.status_code == 200) {
            auto responseJson = nlohmann::json::parse(response.text);
            if (responseJson.contains("chunk")) {
                DEBUG_MSG("Found " + std::to_string(responseJson["chunk"].size()) + " room members");
                
                // Store room members for key sharing
                m_vRoomMembers.clear();
                for (const auto& member : responseJson["chunk"]) {
                    if (member.contains("content") && member["content"].contains("membership") &&
                        member["content"]["membership"] == "join" && member.contains("state_key")) {
                        std::string user_id = member["state_key"];
                        m_vRoomMembers.push_back(user_id);
                        DEBUG_MSG("Added room member: " + user_id);
                    }
                }
                DEBUG_MSG("Stored " + std::to_string(m_vRoomMembers.size()) + " active room members");
                return true;
            }
        } else {
            DEBUG_MSG("Failed to get room members: " + std::to_string(response.status_code));
        }
        
        return false;
    } catch (const std::exception& e) {
        (void)e;
        DEBUG_MSG("Room members error: " + std::string(e.what()));
        return false;
    }
}

bool CChat::HttpClaimKeys(const std::vector<std::string>& user_ids)
{
    EnsureInitialized();
    
    try {
        nlohmann::json claimData = {
            {"one_time_keys", {}}
        };
        
        // Claim one-time keys for each user's devices
        for (const auto& user_id : user_ids) {
            // Get devices for this user from crypto
            auto devices = m_pCrypto->GetUserDevices(user_id);
            if (devices.empty()) {
                SDK::Output("Matrix", ("No devices found for user: " + user_id).c_str(), { 255, 200, 100, 255 });
                continue;
            }
            
            nlohmann::json userDevices;
            for (const auto& [device_id, device] : devices) {
                userDevices[device_id] = "signed_curve25519";
            }
            claimData["one_time_keys"][user_id] = userDevices;
            
            SDK::Output("Matrix", ("Claiming keys for " + std::to_string(devices.size()) + " devices of user: " + user_id).c_str(), { 200, 200, 200, 255 });
        }
        
        std::map<std::string, std::string> headers = {
            {"Authorization", "Bearer " + m_sAccessToken},
            {"Content-Type", "application/json"}
        };
        
        std::string url = m_sBaseUrl + "/_matrix/client/v3/keys/claim";
        auto response = HttpClient::Post(url, claimData.dump(), headers);
        
        if (response.status_code == 200) {
            auto responseJson = nlohmann::json::parse(response.text);
            SDK::Output("Matrix", ("Keys claim response: " + response.text.substr(0, 200) + "...").c_str(), { 200, 200, 200, 255 });
            
            if (responseJson.contains("one_time_keys")) {
                auto& one_time_keys = responseJson["one_time_keys"];
                SDK::Output("Matrix", ("Claimed one-time keys for " + std::to_string(one_time_keys.size()) + " users").c_str(), { 150, 255, 150, 255 });
                
                // Process claimed keys and store them for session creation
                for (auto& [userId, devices] : one_time_keys.items()) {
                    SDK::Output("Matrix", ("Processing keys for user: " + userId).c_str(), { 200, 255, 200, 255 });
                    for (auto& [deviceId, keys] : devices.items()) {
                        SDK::Output("Matrix", ("Processing device: " + deviceId).c_str(), { 200, 200, 200, 255 });
                        for (auto& [keyId, keyValue] : keys.items()) {
                            // Extract the actual key from the signed object
                            std::string actualKey;
                            if (keyValue.is_object() && keyValue.contains("key")) {
                                actualKey = keyValue["key"];
                            } else {
                                actualKey = keyValue; // fallback for simple string keys
                            }
                            
                            std::string deviceKey = userId + ":" + deviceId;
                            m_pCrypto->StoreClaimedKey(deviceKey, actualKey);
                            SDK::Output("Matrix", ("Stored claimed key " + keyId + " from " + userId + ":" + deviceId).c_str(), { 150, 255, 150, 255 });
                        }
                    }
                }
            } else {
                SDK::Output("Matrix", "No one_time_keys field in response", { 255, 150, 150, 255 });
            }
            return true;
        } else {
            DEBUG_MSG("Failed to claim keys: " + std::to_string(response.status_code));
        }
        
        return false;
    } catch (const std::exception& e) {
        (void)e;
        DEBUG_MSG("Key claim error: " + std::string(e.what()));
        return false;
    }
}

bool CChat::InitializeEncryption()
{
    EnsureInitialized();
    
    SDK::Output("Matrix", "=== ENCRYPTION INITIALIZATION DEBUG ===", { 255, 255, 100, 255 });
    
    if (!m_pCrypto) {
        SDK::Output("Matrix", "ERROR: m_pCrypto is NULL", { 255, 150, 150, 255 });
        return false;
    }
    
    SDK::Output("Matrix", "Crypto object exists, initializing...", { 200, 200, 200, 255 });
    
    // Initialize crypto with our user ID
    std::string userId = "@" + Vars::Chat::Username.Value + ":" + Vars::Chat::Server.Value;
    SDK::Output("Matrix", ("Initializing encryption for user: " + userId).c_str(), { 200, 200, 200, 255 });
    
    bool success = m_pCrypto->Initialize(userId);
    SDK::Output("Matrix", ("Crypto Initialize() returned: " + std::string(success ? "SUCCESS" : "FAILED")).c_str(), 
                success ? Color_t{150, 255, 150} : Color_t{255, 150, 150});
    
    if (success) {
        m_bEncryptionEnabled = true;
        SDK::Output("Matrix", "Global encryption ENABLED", { 150, 255, 150, 255 });
        QueueMessage("Matrix", "Encryption initialized for device: " + m_pCrypto->GetDeviceId());
        
        // Follow Element-web initialization sequence: device validation -> key upload -> device keys download
        
        // Step 1: Validate our device credentials before proceeding (Element-web pattern)
        std::string userId = "@" + Vars::Chat::Username.Value + ":" + Vars::Chat::Server.Value;
        if (m_sAccessToken.empty() || userId.empty() || Vars::Chat::Username.Value.empty()) {
            SDK::Output("Matrix", "Device validation failed - missing credentials", { 255, 150, 150, 255 });
            return false;
        }
        
        // Step 2: Upload device keys and one-time keys together (Element-web compatible method)
        if (HttpUploadInitialKeys()) {
            SDK::Output("Matrix", "Device and one-time keys uploaded successfully", { 150, 255, 150, 255 });
        } else {
            SDK::Output("Matrix", "Failed to upload initial keys", { 255, 150, 150, 255 });
            // Continue anyway for compatibility
        }
        
        // Step 3: Get room members for key sharing (must be done before downloading device keys)
        HttpGetRoomMembers();
        
        // Download device keys for other users in the room
        HttpDownloadDeviceKeys();
        
    } else {
        SDK::Output("Matrix", "ERROR: Failed to initialize encryption - using unencrypted mode", { 255, 150, 150, 255 });
        QueueMessage("Matrix", "Failed to initialize encryption - using unencrypted mode");
    }
    
    return success;
}

void CChat::SetRoomEncryption(const std::string& room_id, bool encrypted)
{
    std::lock_guard<std::mutex> lock(m_EncryptionStateMutex);
    m_RoomEncryptionState[room_id] = encrypted;
    
    if (encrypted) {
        QueueMessage("Matrix", "Room encryption enabled for " + room_id);
    } else {
        QueueMessage("Matrix", "Room encryption disabled for " + room_id);
    }
}

bool CChat::IsRoomEncrypted(const std::string& room_id) const
{
    std::lock_guard<std::mutex> lock(m_EncryptionStateMutex);
    auto it = m_RoomEncryptionState.find(room_id);
    bool result = it != m_RoomEncryptionState.end() && it->second;
    
    // Debug output to track room encryption state
    DEBUG_MSG("IsRoomEncrypted check for room: " + room_id);
    DEBUG_MSG("Room found in state map: " + std::string(it != m_RoomEncryptionState.end() ? "YES" : "NO"));
    if (it != m_RoomEncryptionState.end()) {
        DEBUG_MSG("Room encryption value: " + std::string(it->second ? "TRUE" : "FALSE"));
    }
    DEBUG_MSG("Total rooms in encryption state map: " + std::to_string(m_RoomEncryptionState.size()));
    
    return result;
}

bool CChat::ShouldEncryptMessage(const std::string& room_id) const
{
    // Only encrypt if both global encryption is enabled AND the room is encrypted
    return IsEncryptionEnabled() && IsRoomEncrypted(room_id);
}

std::string CChat::SimpleEncrypt(const std::string& text)
{
    // For local config storage, just return the text as-is (plaintext)
    // This is simpler and more reliable than complex encryption
    return text;
}

std::string CChat::SimpleDecrypt(const std::string& encrypted)
{
    // For local config storage, just return the text as-is (plaintext)  
    // This matches the SimpleEncrypt behavior
    return encrypted;
}

std::string CChat::EncryptPassword(const std::string& password)
{
    return SimpleEncrypt(password);
}

std::string CChat::DecryptPassword(const std::string& encryptedPassword)
{
    return SimpleDecrypt(encryptedPassword);
}

bool CChat::HttpUploadInitialKeys()
{
    EnsureInitialized();
    
    if (!m_pCrypto) {
        DEBUG_MSG("Cannot upload initial keys - encryption not initialized");
        return false;
    }
    
    try {
        // Get device keys
        std::string deviceKeysJson = m_pCrypto->GetDeviceKeys();
        if (deviceKeysJson.empty()) {
            DEBUG_MSG("Failed to get device keys");
            return false;
        }
        
        // Get one-time keys
        std::string oneTimeKeysJson = m_pCrypto->GetOneTimeKeys();
        if (oneTimeKeysJson.empty()) {
            DEBUG_MSG("Failed to get one-time keys");
            return false;
        }
        
        // Parse both key sets
        auto deviceKeys = nlohmann::json::parse(deviceKeysJson);
        auto oneTimeKeys = nlohmann::json::parse(oneTimeKeysJson);
        
        // Create combined upload data exactly matching Element-web's IUploadKeysRequest
        nlohmann::json uploadData = {
            {"device_keys", deviceKeys},
            {"one_time_keys", oneTimeKeys}
        };
        
        std::map<std::string, std::string> headers = {
            {"Authorization", "Bearer " + m_sAccessToken},
            {"Content-Type", "application/json"}
        };
        
        std::string url = m_sBaseUrl + "/_matrix/client/v3/keys/upload";
        auto response = HttpClient::Post(url, uploadData.dump(), headers);
        
        if (response.status_code == 200) {
            // Mark one-time keys as published
            m_pCrypto->MarkOneTimeKeysAsPublished();
            
            // Parse response to get key counts (Element-web compatible)
            try {
                auto responseJson = nlohmann::json::parse(response.text);
                if (responseJson.contains("one_time_key_counts")) {
                    auto& keyCounts = responseJson["one_time_key_counts"];
                    if (keyCounts.contains("signed_curve25519")) {
                        int keyCount = keyCounts["signed_curve25519"];
                        DEBUG_MSG("Server now has " + std::to_string(keyCount) + " one-time keys");
                    }
                }
            } catch (const std::exception& e) {
                (void)e; // Suppress unused variable warning in release builds
                DEBUG_MSG("Could not parse key upload response: " + std::string(e.what()));
            }
            
            DEBUG_MSG("Initial keys (device + one-time) uploaded successfully");
            return true;
        } else {
            std::string errorMsg = "Failed to upload initial keys (HTTP " + std::to_string(response.status_code) + ")";
            if (response.status_code == 401) {
                errorMsg += " - Authentication failed";
            } else if (response.status_code == 403) {
                errorMsg += " - Forbidden";
            }
            if (!response.text.empty()) {
                errorMsg += " - " + response.text;
            }
            SDK::Output("Matrix", errorMsg.c_str(), { 255, 150, 150, 255 });
            DEBUG_MSG(errorMsg);
            return false;
        }
    } catch (const std::exception& e) {
        std::string errorMsg = "Initial key upload error: " + std::string(e.what());
        SDK::Output("Matrix", errorMsg.c_str(), { 255, 150, 150, 255 });
        DEBUG_MSG(errorMsg);
        return false;
    }
}

void CChat::SendPendingRoomKeys()
{
    if (!m_pCrypto || !IsEncryptionEnabled()) {
        return;
    }
    
    // Get all pending room keys from the crypto system
    auto pending_keys = m_pCrypto->GetAndClearPendingRoomKeys();
    
    if (pending_keys.empty()) {
        return;
    }
    
    SDK::Output("Matrix", ("Sending " + std::to_string(pending_keys.size()) + " pending room keys").c_str(), { 150, 255, 150, 255 });
    
    for (const auto& key : pending_keys) {
        try {
            if (key.encrypted_content.empty() || key.encrypted_content == "{}") {
                SDK::Output("Matrix", ("Skipping empty room key for " + key.user_id + ":" + key.device_id).c_str(), { 255, 150, 150, 255 });
                continue;
            }
            
            // The encrypted_content contains the full m.room.encrypted event
            // Send it as a to-device message
            bool success = HttpSendToDevice("m.room.encrypted", key.user_id, key.device_id, key.encrypted_content);
            
            if (success) {
                SDK::Output("Matrix", ("Sent room key to " + key.user_id + ":" + key.device_id).c_str(), { 150, 255, 150, 255 });
            } else {
                SDK::Output("Matrix", ("Failed to send room key to " + key.user_id + ":" + key.device_id).c_str(), { 255, 150, 150, 255 });
            }
            
        } catch (const std::exception& e) {
            SDK::Output("Matrix", ("Error sending room key: " + std::string(e.what())).c_str(), { 255, 150, 150, 255 });
        }
    }
}

bool CChat::HttpSendRoomKeyRequest(const std::string& room_id, const std::string& session_id, const std::string& sender_key)
{
    EnsureInitialized();
    
    if (!m_pCrypto) {
        DEBUG_MSG("Cannot send room key request - crypto not initialized");
        return false;
    }
    
    try {
        // Create room key request exactly matching Element-web format
        // This matches the m.room_key_request event structure
        std::string request_id = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
        
        nlohmann::json request_content = {
            {"action", "request"},
            {"body", {
                {"algorithm", "m.megolm.v1.aes-sha2"},
                {"room_id", room_id},
                {"session_id", session_id},
                {"sender_key", sender_key}
            }},
            {"request_id", request_id},
            {"requesting_device_id", m_pCrypto->GetDeviceId()}
        };
        
        // Send to all devices of all users in the room (Element-web pattern)
        bool success = true;
        for (const auto& member : m_vRoomMembers) {
            if (!HttpSendToDevice("m.room_key_request", member, "*", request_content.dump())) {
                success = false;
                DEBUG_MSG("Failed to send room key request to " + member);
            }
        }
        
        if (success) {
            DEBUG_MSG("Room key request sent successfully for session " + session_id);
        }
        
        return success;
        
    } catch (const std::exception& e) {
        (void)e; // Suppress unused variable warning in release builds
        DEBUG_MSG("Room key request error: " + std::string(e.what()));
        return false;
    }
}

