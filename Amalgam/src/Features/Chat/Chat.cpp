#include "Chat.h"
#include "../Visuals/MarkSpot/MarkSpot.h"
#include <chrono>
#include <iomanip>
#include <atomic>
#include <ctime>
#include <fstream>
#include <cstdio>
#include <algorithm>

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


CChat::CChat()
{
    // Load chat settings on startup
    LoadChatSettings();
    
    // Initialize encryption system
    m_pCrypto = std::make_unique<MatrixCrypto>();
    
    // Set global instance pointer for HTTP logging
    g_ChatInstance = this;
    
    // Set up HTTP logging callback to use TF2 console
    HttpClient::SetLogCallback([](const std::string& prefix, const std::string& message) {
        // Access the global chat instance to queue messages
        if (g_ChatInstance) {
            g_ChatInstance->QueueMessage(prefix, message);
        }
    });
    
    
    // Auto-connect if enabled
    if (Vars::Chat::AutoConnect.Value)
    {
        Connect();
    }
}

CChat::~CChat()
{
    // Clear global instance pointer
    g_ChatInstance = nullptr;
    
    // Save chat settings on shutdown
    SaveChatSettings();
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
    
    // Don't show our own messages twice
    if (senderName != Vars::Chat::Username.Value)
    {
        // Queue the message for display on the main thread instead of calling ChatPrintf directly
        // ChatPrintf is not thread-safe and crashes when called from background threads
        QueueMessage("[Matrix] " + senderName, content);
    }
}

void CChat::QueueMessage(const std::string& sender, const std::string& content)
{
    std::lock_guard<std::mutex> lock(m_MessageQueueMutex);
    m_MessageQueue.push({sender, content});
}

void CChat::ProcessQueuedMessages()
{
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
                auto tm = *std::localtime(&time_t);
                
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
        
        // Check if encryption is enabled
        if (IsEncryptionEnabled()) {
            DEBUG_MSG("Encrypting message with Megolm");
            
            // Ensure room session exists first
            if (!m_pCrypto->EnsureRoomSession(m_sRoomId)) {
                DEBUG_MSG("Failed to create/ensure room session");
                content = msgData.dump(); // Fall back to unencrypted
            } else {
                // Share session key with other users in the room
                std::vector<std::string> user_ids = m_vRoomMembers;
            
            // If we don't have room members yet, refresh them
            if (user_ids.empty()) {
                DEBUG_MSG("No room members cached, refreshing...");
                HttpGetRoomMembers();
                user_ids = m_vRoomMembers;
            }
            
            // Ensure we include ourselves if not already in the list
            std::string ourUserId = "@" + Vars::Chat::Username.Value + ":" + Vars::Chat::Server.Value;
            if (std::find(user_ids.begin(), user_ids.end(), ourUserId) == user_ids.end()) {
                user_ids.push_back(ourUserId);
            }
            
            // Ensure we have fresh device keys for all users
            DEBUG_MSG("Downloading device keys for " + std::to_string(user_ids.size()) + " users");
            HttpDownloadDeviceKeys();
            
            // Claim one-time keys for proper Olm session establishment
            HttpClaimKeys(user_ids);
            
            // Share the session key with the users
            m_pCrypto->ShareSessionKey(m_sRoomId, user_ids);
            
            // Send any pending room keys via to-device messages
            auto pendingKeys = m_pCrypto->GetPendingRoomKeys();
            DEBUG_MSG("Sending " + std::to_string(pendingKeys.size()) + " room key to-device messages");
            for (const auto& key : pendingKeys) {
                HttpSendToDevice("m.room.encrypted", key.user_id, key.device_id, key.encrypted_content);
            }
            m_pCrypto->ClearPendingRoomKeys();
            
            // Brief delay to allow to-device messages to be processed
            if (!pendingKeys.empty()) {
                DEBUG_MSG("Waiting for to-device messages to be processed...");
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
            
            // Encrypt the message content
            std::string encryptedContent = m_pCrypto->EncryptMessage(m_sRoomId, eventType, msgData.dump());
            if (!encryptedContent.empty()) {
                eventType = "m.room.encrypted";
                content = encryptedContent;
                DEBUG_MSG("Message encrypted successfully");
                } else {
                    DEBUG_MSG("Encryption failed, sending unencrypted");
                    content = msgData.dump();
                }
            }
        } else {
            // Encryption disabled, send unencrypted message
            content = msgData.dump();
            DEBUG_MSG("Sending unencrypted message");
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
            DEBUG_MSG("Message sent successfully!");
            
            // Don't show local echo - we'll receive it back from the server with proper formatting
            
            return true;
        }
        else
        {
            DEBUG_MSG("Send failed with status: " + std::to_string(response.status_code));
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
            
            // Process events from our target room only
            if (json.contains("rooms") && json["rooms"].contains("join"))
            {
                for (const auto& [roomId, roomData] : json["rooms"]["join"].items())
                {
                    // Only process messages from our target room
                    if (roomId == m_sRoomId && roomData.contains("timeline") && roomData["timeline"].contains("events"))
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
                                if (event.contains("content")) {
                                    content = m_pCrypto->DecryptMessage(roomId, event["content"].dump());
                                    if (!content.empty() && !content.starts_with("[")) {
                                        DEBUG_MSG("Decrypted message: " + content);
                                        processed = true;
                                    } else {
                                        DEBUG_MSG("Decryption failed: " + content);
                                    }
                                } else {
                                    DEBUG_MSG("Encrypted event missing content");
                                }
                            }
                            // Handle unencrypted text messages
                            else if (event.contains("type") && event["type"] == "m.room.message" && 
                                     event.contains("content") && event["content"].contains("msgtype") && 
                                     event["content"]["msgtype"] == "m.text")
                            {
                                content = event["content"]["body"];
                                DEBUG_MSG("Unencrypted message from " + sender + ": " + content);
                                processed = true;
                            }
                            
                            if (processed && !content.empty()) {
                                ProcessIncomingMessage(sender, content);
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
        
        nlohmann::json uploadData = {
            {"device_keys", nlohmann::json::parse(deviceKeysJson)}
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
            DEBUG_MSG("Failed to upload device keys: " + std::to_string(response.status_code) + " - " + response.text);
            return false;
        }
    } catch (const std::exception& e) {
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
        DEBUG_MSG("Device key download error: " + std::string(e.what()));
        return false;
    }
}

bool CChat::HttpSendToDevice(const std::string& event_type, const std::string& target_user, const std::string& target_device, const std::string& content)
{
    try {
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
        
        // Generate transaction ID
        static int txnCounter = 0;
        std::string txnId = "txn" + std::to_string(++txnCounter);
        
        std::string url = m_sBaseUrl + "/_matrix/client/v3/sendToDevice/" + event_type + "/" + txnId;
        auto response = HttpClient::Put(url, sendData.dump(), headers);
        
        if (response.status_code == 200) {
            DEBUG_MSG("To-device message sent: " + event_type);
            return true;
        } else {
            DEBUG_MSG("Failed to send to-device message: " + std::to_string(response.status_code));
            return false;
        }
    } catch (const std::exception& e) {
        DEBUG_MSG("To-device send error: " + std::string(e.what()));
        return false;
    }
}

bool CChat::HttpUploadOneTimeKeys()
{
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
            DEBUG_MSG("Failed to upload one-time keys: " + std::to_string(response.status_code));
            return false;
        }
    } catch (const std::exception& e) {
        DEBUG_MSG("One-time key upload error: " + std::string(e.what()));
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
        DEBUG_MSG("Room members error: " + std::string(e.what()));
        return false;
    }
}

bool CChat::HttpClaimKeys(const std::vector<std::string>& user_ids)
{
    try {
        nlohmann::json claimData = {
            {"one_time_keys", {}}
        };
        
        // Claim one-time keys for each user's devices
        for (const auto& user_id : user_ids) {
            claimData["one_time_keys"][user_id] = {
                {"*", "curve25519"}  // Claim curve25519 keys from any device
            };
        }
        
        std::map<std::string, std::string> headers = {
            {"Authorization", "Bearer " + m_sAccessToken},
            {"Content-Type", "application/json"}
        };
        
        std::string url = m_sBaseUrl + "/_matrix/client/v3/keys/claim";
        auto response = HttpClient::Post(url, claimData.dump(), headers);
        
        if (response.status_code == 200) {
            auto responseJson = nlohmann::json::parse(response.text);
            if (responseJson.contains("one_time_keys")) {
                DEBUG_MSG("Claimed one-time keys for " + std::to_string(responseJson["one_time_keys"].size()) + " users");
                
                // Process claimed keys and store them for session creation
                for (auto& [userId, devices] : responseJson["one_time_keys"].items()) {
                    for (auto& [deviceId, keys] : devices.items()) {
                        for (auto& [keyId, keyValue] : keys.items()) {
                            // We need to map the claimed one-time key to the device's curve25519 key
                            // The one-time key should be stored using the device's curve25519 key as the map key
                            // For now, store with device ID as key - this will be resolved in the crypto layer
                            std::string deviceKey = userId + ":" + deviceId;
                            m_pCrypto->StoreClaimedKey(deviceKey, keyValue);
                            DEBUG_MSG("Stored claimed key " + keyId + " from " + userId + ":" + deviceId);
                        }
                    }
                }
                return true;
            }
        } else {
            DEBUG_MSG("Failed to claim keys: " + std::to_string(response.status_code));
        }
        
        return false;
    } catch (const std::exception& e) {
        DEBUG_MSG("Key claim error: " + std::string(e.what()));
        return false;
    }
}

bool CChat::InitializeEncryption()
{
    if (!m_pCrypto) {
        return false;
    }
    
    // Initialize crypto with our user ID
    bool success = m_pCrypto->Initialize("@" + Vars::Chat::Username.Value + ":" + Vars::Chat::Server.Value);
    if (success) {
        m_bEncryptionEnabled = true;
        QueueMessage("Matrix", "Encryption initialized for device: " + m_pCrypto->GetDeviceId());
        
        // Upload our device keys to the server
        if (HttpUploadDeviceKeys()) {
            DEBUG_MSG("Device keys uploaded successfully");
        } else {
            DEBUG_MSG("Failed to upload device keys");
        }
        
        // Upload one-time keys for session establishment
        if (HttpUploadOneTimeKeys()) {
            DEBUG_MSG("One-time keys uploaded successfully");
        } else {
            DEBUG_MSG("Failed to upload one-time keys");
        }
        
        // Get room members for key sharing (must be done before downloading device keys)
        HttpGetRoomMembers();
        
        // Download device keys for other users in the room
        HttpDownloadDeviceKeys();
        
    } else {
        QueueMessage("Matrix", "Failed to initialize encryption - using unencrypted mode");
    }
    
    return success;
}

void CChat::SaveChatSettings()
{
    try
    {
        nlohmann::json chatSettings;
        
        // Always save non-sensitive settings
        chatSettings["space"] = Vars::Chat::Space.Value;
        chatSettings["room"] = Vars::Chat::Room.Value;
        chatSettings["auto_connect"] = Vars::Chat::AutoConnect.Value;
        chatSettings["show_timestamps"] = Vars::Chat::ShowTimestamps.Value;
        chatSettings["save_credentials"] = Vars::Chat::SaveCredentials.Value;
        
        // Only save credentials if user has enabled the option
        if (Vars::Chat::SaveCredentials.Value)
        {
            chatSettings["server"] = Vars::Chat::Server.Value;
            chatSettings["username"] = Vars::Chat::Username.Value;
            chatSettings["email"] = Vars::Chat::Email.Value;
            
            // Basic XOR encryption for password (better than plain text)
            std::string encryptedPassword = SimpleEncrypt(Vars::Chat::Password.Value);
            chatSettings["password_encrypted"] = encryptedPassword;
        }
        
        std::ofstream file("chat_settings.json");
        if (file.is_open())
        {
            file << chatSettings.dump(4);
            file.close();
        }
    }
    catch (const std::exception&)
    {
        // Silently ignore save errors
    }
}

void CChat::LoadChatSettings()
{
    try
    {
        std::ifstream file("chat_settings.json");
        if (file.is_open())
        {
            nlohmann::json chatSettings;
            file >> chatSettings;
            file.close();
            
            // Always load non-sensitive settings
            if (chatSettings.contains("space"))
                Vars::Chat::Space.Value = chatSettings["space"].get<std::string>();
            if (chatSettings.contains("room"))
                Vars::Chat::Room.Value = chatSettings["room"].get<std::string>();
            if (chatSettings.contains("auto_connect"))
                Vars::Chat::AutoConnect.Value = chatSettings["auto_connect"].get<bool>();
            if (chatSettings.contains("show_timestamps"))
                Vars::Chat::ShowTimestamps.Value = chatSettings["show_timestamps"].get<bool>();
            if (chatSettings.contains("save_credentials"))
                Vars::Chat::SaveCredentials.Value = chatSettings["save_credentials"].get<bool>();
            
            // Load credentials if they were saved
            if (Vars::Chat::SaveCredentials.Value)
            {
                if (chatSettings.contains("server"))
                    Vars::Chat::Server.Value = chatSettings["server"].get<std::string>();
                if (chatSettings.contains("username"))
                    Vars::Chat::Username.Value = chatSettings["username"].get<std::string>();
                if (chatSettings.contains("email"))
                    Vars::Chat::Email.Value = chatSettings["email"].get<std::string>();
                
                // Load encrypted password
                if (chatSettings.contains("password_encrypted"))
                {
                    std::string encryptedPassword = chatSettings["password_encrypted"].get<std::string>();
                    Vars::Chat::Password.Value = SimpleDecrypt(encryptedPassword);
                }
                // Legacy support for old plain text passwords
                else if (chatSettings.contains("password"))
                {
                    Vars::Chat::Password.Value = chatSettings["password"].get<std::string>();
                }
            }
        }
    }
    catch (const std::exception&)
    {
        // Silently ignore load errors - use defaults
    }
}

std::string CChat::SimpleEncrypt(const std::string& text)
{
    if (text.empty()) return "";
    
    try
    {
        // Use Olm PK encryption for cryptographically secure credential storage
        size_t encryption_size = olm_pk_encryption_size();
        std::vector<uint8_t> encryption_memory(encryption_size);
        OlmPkEncryption* encryption = olm_pk_encryption(encryption_memory.data());
        
        if (!encryption) return "";
        
        // Generate a deterministic but secure key from system info
        // This creates a unique key per machine/user while being reproducible
        std::string system_entropy = GetSystemEntropy();
        
        // Use first 32 bytes as public key (Curve25519 key length)
        if (system_entropy.length() < 32) return "";
        
        // Set recipient key (ourselves)
        size_t key_result = olm_pk_encryption_set_recipient_key(
            encryption, 
            system_entropy.data(), 
            32
        );
        
        if (key_result == olm_error()) {
            olm_clear_pk_encryption(encryption);
            return "";
        }
        
        // Calculate buffer sizes
        size_t ciphertext_length = olm_pk_ciphertext_length(encryption, text.length());
        size_t mac_length = olm_pk_mac_length(encryption);
        size_t ephemeral_key_length = olm_pk_key_length();
        size_t random_length = olm_pk_encrypt_random_length(encryption);
        
        // Prepare buffers
        std::vector<uint8_t> ciphertext(ciphertext_length);
        std::vector<uint8_t> mac(mac_length);
        std::vector<uint8_t> ephemeral_key(ephemeral_key_length);
        std::vector<uint8_t> random_data(random_length);
        
        // Generate cryptographically secure random data
        if (!GenerateSecureRandom(random_data.data(), random_length)) {
            olm_clear_pk_encryption(encryption);
            return "";
        }
        
        // Encrypt
        size_t result = olm_pk_encrypt(
            encryption,
            text.data(), text.length(),
            ciphertext.data(), ciphertext_length,
            mac.data(), mac_length,
            ephemeral_key.data(), ephemeral_key_length,
            random_data.data(), random_length
        );
        
        olm_clear_pk_encryption(encryption);
        
        if (result == olm_error()) return "";
        
        // Combine all parts into base64 encoded string
        std::string combined;
        combined.append(reinterpret_cast<char*>(ciphertext.data()), ciphertext_length);
        combined.append(reinterpret_cast<char*>(mac.data()), mac_length);
        combined.append(reinterpret_cast<char*>(ephemeral_key.data()), ephemeral_key_length);
        
        return Base64Encode(combined);
    }
    catch (const std::exception&)
    {
        return "";
    }
}

std::string CChat::SimpleDecrypt(const std::string& encrypted)
{
    if (encrypted.empty()) return "";
    
    try
    {
        // Decode base64
        std::string combined = Base64Decode(encrypted);
        if (combined.empty()) return "";
        
        // Use Olm PK decryption
        size_t decryption_size = olm_pk_decryption_size();
        std::vector<uint8_t> decryption_memory(decryption_size);
        OlmPkDecryption* decryption = olm_pk_decryption(decryption_memory.data());
        
        if (!decryption) return "";
        
        // Generate the same deterministic key
        std::string system_entropy = GetSystemEntropy();
        if (system_entropy.length() < 64) return ""; // Need 32 bytes for private key + 32 for public
        
        // Extract private key (next 32 bytes after public key)
        std::vector<uint8_t> private_key(system_entropy.begin() + 32, system_entropy.begin() + 64);
        std::vector<uint8_t> public_key_buffer(olm_pk_key_length());
        
        // Initialize decryption with private key
        size_t key_result = olm_pk_key_from_private(
            decryption,
            public_key_buffer.data(), public_key_buffer.size(),
            private_key.data(), private_key.size()
        );
        
        if (key_result == olm_error()) {
            olm_clear_pk_decryption(decryption);
            return "";
        }
        
        // Calculate component sizes
        size_t mac_length = olm_pk_mac_length(nullptr); // Same for encrypt/decrypt
        size_t ephemeral_key_length = olm_pk_key_length();
        
        if (combined.length() < mac_length + ephemeral_key_length) {
            olm_clear_pk_decryption(decryption);
            return "";
        }
        
        // Extract components
        size_t ciphertext_length = combined.length() - mac_length - ephemeral_key_length;
        uint8_t* ciphertext = reinterpret_cast<uint8_t*>(const_cast<char*>(combined.data()));
        uint8_t* mac = ciphertext + ciphertext_length;
        uint8_t* ephemeral_key = mac + mac_length;
        
        // Decrypt
        size_t max_plaintext_length = olm_pk_max_plaintext_length(decryption, ciphertext_length);
        std::vector<uint8_t> plaintext(max_plaintext_length);
        
        size_t result = olm_pk_decrypt(
            decryption,
            ephemeral_key, ephemeral_key_length,
            mac, mac_length,
            ciphertext, ciphertext_length,
            plaintext.data(), max_plaintext_length
        );
        
        olm_clear_pk_decryption(decryption);
        
        if (result == olm_error()) return "";
        
        return std::string(reinterpret_cast<char*>(plaintext.data()), result);
    }
    catch (const std::exception&)
    {
        return "";
    }
}

std::string CChat::GetSystemEntropy()
{
    // Generate deterministic but unique entropy from system characteristics
    // This creates a machine-specific key that's reproducible
    std::string entropy;
    
    // Add various system identifiers to create unique entropy
    entropy += "AmalgamChat_v1.0_";
    
    // Windows-specific entropy
    char computerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(computerName);
    if (GetComputerNameA(computerName, &size)) {
        entropy += computerName;
    }
    
    char userName[UNLEN + 1];
    size = sizeof(userName);
    if (GetUserNameA(userName, &size)) {
        entropy += "_";
        entropy += userName;
    }
    
    // Add application-specific data
    entropy += "_TF2_Amalgam";
    
    // Hash the entropy to get consistent 64 bytes
    // Using a simple hash function to create deterministic output
    std::string hashed(64, '\0');
    for (size_t i = 0; i < 64; ++i) {
        uint8_t byte = 0;
        for (size_t j = 0; j < entropy.length(); ++j) {
            byte ^= static_cast<uint8_t>(entropy[j] + i + j);
        }
        hashed[i] = static_cast<char>(byte);
    }
    
    return hashed;
}

bool CChat::GenerateSecureRandom(uint8_t* buffer, size_t length)
{
    if (!buffer || length == 0) return false;
    
    // Use Windows CryptGenRandom for secure random numbers
    HCRYPTPROV hProv;
    if (!CryptAcquireContextA(&hProv, nullptr, nullptr, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        return false;
    }
    
    BOOL result = CryptGenRandom(hProv, static_cast<DWORD>(length), buffer);
    CryptReleaseContext(hProv, 0);
    return result == TRUE;
}

std::string CChat::Base64Encode(const std::string& input)
{
    static const char base64_chars[] = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
    std::string encoded;
    int val = 0, valb = -6;
    for (unsigned char c : input) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            encoded.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) encoded.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    while (encoded.size() % 4) encoded.push_back('=');
    return encoded;
}

std::string CChat::Base64Decode(const std::string& input)
{
    static const int decode_table[128] = {
        -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
        -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
        -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,62, -1,-1,-1,63,
        52,53,54,55, 56,57,58,59, 60,61,-1,-1, -1,-1,-1,-1,
        -1, 0, 1, 2,  3, 4, 5, 6,  7, 8, 9,10, 11,12,13,14,
        15,16,17,18, 19,20,21,22, 23,24,25,-1, -1,-1,-1,-1,
        -1,26,27,28, 29,30,31,32, 33,34,35,36, 37,38,39,40,
        41,42,43,44, 45,46,47,48, 49,50,51,-1, -1,-1,-1,-1
    };
    
    std::string decoded;
    int val = 0, valb = -8;
    for (unsigned char c : input) {
        if (c > 127 || decode_table[c] == -1) continue;
        val = (val << 6) + decode_table[c];
        valb += 6;
        if (valb >= 0) {
            decoded.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return decoded;
}