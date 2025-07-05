#include "Chat.h"
#include <chrono>
#include <iomanip>
#include <atomic>
#include <ctime>
#include <fstream>
#include <cstdio>

CChat::CChat()
{
    // Load chat settings on startup
    LoadChatSettings();
    
    // Initialize encryption system
    m_pCrypto = std::make_unique<MatrixCrypto>();
    
    // Auto-connect if enabled
    if (Vars::Chat::AutoConnect.Value)
    {
        Connect();
    }
}

CChat::~CChat()
{
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
        QueueMessage("Matrix Debug", "Testing connectivity to: " + m_sBaseUrl);
        auto versionResponse = HttpClient::Get(m_sBaseUrl + "/_matrix/client/versions", {});
        QueueMessage("Matrix Debug", "Server versions check status: " + std::to_string(versionResponse.status_code));
        
        if (versionResponse.status_code == 0)
        {
            QueueMessage("Matrix Debug", "Network connectivity failed: " + versionResponse.text);
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
                CreateAccount(username, password);
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
}

void CChat::CreateAccount(const std::string& username, const std::string& password)
{
    try
    {
        // First, get the available auth flows from the server
        auto flowResponse = HttpClient::Post(
            "https://" + Vars::Chat::Server.Value + "/_matrix/client/v3/register",
            "{}",
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
            flows = flowJson["flows"];
        }
        
        // Try to register with dummy auth (most common for open servers)
        nlohmann::json registerData = {
            {"username", username},
            {"password", password},
            {"device_id", "AmalgamClient"},
            {"inhibit_login", false}
        };
        
        if (!sessionId.empty())
        {
            registerData["auth"] = {
                {"type", "m.login.dummy"},
                {"session", sessionId}
            };
        }
        else
        {
            registerData["auth"] = {
                {"type", "m.login.dummy"}
            };
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
        QueueMessage("Matrix Debug", "Found !! prefix, processing Matrix message");
        
        std::string chatMessage = trimmedMessage.substr(2); // Remove !! prefix
        
        // Trim whitespace
        while (!chatMessage.empty() && (chatMessage.front() == ' ' || chatMessage.front() == '\t'))
            chatMessage.erase(0, 1);
        
        if (!chatMessage.empty())
        {
            QueueMessage("Matrix Debug", "Sending: " + chatMessage);
            
            // Check if we're connected before sending
            if (!m_bConnected.load())
            {
                QueueMessage("Matrix", "Not connected to Matrix server");
                return true; // Still block from game chat
            }
            
            SendMessage(chatMessage);
            QueueMessage("Matrix Debug", "Blocking message from game chat");
            return true; // Message was processed and should be blocked from game chat
        }
        else
        {
            QueueMessage("Matrix Debug", "Empty message after trimming, blocking from game chat");
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
    
    // Don't show our own messages twice
    if (senderName != Vars::Chat::Username.Value)
    {
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
    // Only output to console if debug logging is enabled, otherwise just in-game chat
    if (sender.find("Debug") != std::string::npos)
    {
        // Debug messages only go to console/stdout when debug is enabled
        printf("[Matrix] %s: %s\n", sender.c_str(), content.c_str());
        return;
    }
    
    // Print regular Matrix messages to TF2's console with color
    if (I::CVar)
    {
        Color_t tColor = { 0, 255, 255, 255 }; // Cyan color for Matrix messages
        I::CVar->ConsoleColorPrintf(tColor, "[Matrix] %s: %s\n", sender.c_str(), content.c_str());
    }
    
    // DISABLED: In-game chat display for Matrix messages due to crashes
    // Matrix messages will only appear in console to prevent game crashes
    // This is safer and more reliable than trying to use TF2's chat system
    // which can be unstable when accessed from external code
    
    // Note: Users can still see Matrix messages in the console (cyan color)
    // If in-game display is needed in the future, it should be implemented
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
            QueueMessage("Matrix Debug", "Login successful, access token obtained");
            return true;
        }
        else
        {
            std::lock_guard<std::mutex> lock(m_StatusMutex);
            if (response.status_code == 0)
            {
                m_sLastError = "Network connection failed during login: " + response.text;
                QueueMessage("Matrix Debug", "Login failed with network error (status 0)");
                QueueMessage("Matrix Debug", "Error details: " + response.text);
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
            {"Content-Type", "application/json"},
            {"User-Agent", "Amalgam/1.0 Matrix Client"}
        };
        
        // Ensure base URL is set (in case of race condition)
        if (m_sBaseUrl.empty())
        {
            m_sBaseUrl = "https://" + Vars::Chat::Server.Value;
        }
        
        QueueMessage("Matrix Debug", "Starting room join process...");
        QueueMessage("Matrix Debug", "Space: " + Vars::Chat::Space.Value + ", Room: " + Vars::Chat::Room.Value + ", Server: " + Vars::Chat::Server.Value);
        QueueMessage("Matrix Debug", "Base URL: " + m_sBaseUrl);
        
        // Test basic HTTP connectivity first
        QueueMessage("Matrix Debug", "Testing HTTP connectivity...");
        auto testResponse = HttpClient::Get(m_sBaseUrl + "/_matrix/client/versions", {});
        QueueMessage("Matrix Debug", "Connectivity test result: " + std::to_string(testResponse.status_code));
        QueueMessage("Matrix Debug", "Connectivity test response: " + testResponse.text);
        
        // Strategy 1: Try to resolve and join the space first using directory lookup
        std::string spaceAlias = "#" + Vars::Chat::Space.Value + ":" + Vars::Chat::Server.Value;
        std::string encodedSpaceAlias = HttpClient::UrlEncode(spaceAlias);
        std::string spaceDirectoryUrl = m_sBaseUrl + "/_matrix/client/v3/directory/room/" + encodedSpaceAlias;
        
        QueueMessage("Matrix Debug", "Looking up space in directory: " + spaceAlias);
        QueueMessage("Matrix Debug", "Encoded alias: " + encodedSpaceAlias);
        QueueMessage("Matrix Debug", "Full URL: " + spaceDirectoryUrl);
        
        auto spaceDirectoryResponse = HttpClient::Get(spaceDirectoryUrl, headers);
        
        QueueMessage("Matrix Debug", "Space directory response code: " + std::to_string(spaceDirectoryResponse.status_code));
        QueueMessage("Matrix Debug", "Space directory response: " + spaceDirectoryResponse.text);
        
        if (spaceDirectoryResponse.status_code == 200)
        {
            auto spaceDirectoryJson = nlohmann::json::parse(spaceDirectoryResponse.text);
            std::string spaceRoomId = spaceDirectoryJson["room_id"];
            QueueMessage("Matrix Debug", "Found space in directory: " + spaceRoomId);
            
            // Join the space using room ID
            auto joinSpaceResponse = HttpClient::Post(
                m_sBaseUrl + "/_matrix/client/v3/rooms/" + spaceRoomId + "/join",
                "{}",
                headers
            );
            
            if (joinSpaceResponse.status_code == 200)
            {
                QueueMessage("Matrix Debug", "Successfully joined space: " + spaceRoomId);
                
                // Look for the talk room within the space hierarchy
                auto hierarchyResponse = HttpClient::Get(
                    m_sBaseUrl + "/_matrix/client/v1/rooms/" + spaceRoomId + "/hierarchy",
                    headers
                );
                
                QueueMessage("Matrix Debug", "Space hierarchy response: " + std::to_string(hierarchyResponse.status_code));
                
                if (hierarchyResponse.status_code == 200)
                {
                    auto hierarchyJson = nlohmann::json::parse(hierarchyResponse.text);
                    QueueMessage("Matrix Debug", "Got space hierarchy, looking for room: " + Vars::Chat::Room.Value);
                    QueueMessage("Matrix Debug", "Hierarchy response: " + hierarchyResponse.text);
                    
                    if (hierarchyJson.contains("rooms"))
                    {
                        QueueMessage("Matrix Debug", "Found " + std::to_string(hierarchyJson["rooms"].size()) + " rooms in hierarchy");
                        
                        for (const auto& room : hierarchyJson["rooms"])
                        {
                            std::string roomName = room.contains("name") ? room["name"].get<std::string>() : "unnamed";
                            std::string roomId = room.contains("room_id") ? room["room_id"].get<std::string>() : "unknown";
                            
                            QueueMessage("Matrix Debug", "Hierarchy room: " + roomName + " (" + roomId + ")");
                            
                            if (room.contains("name") && room["name"] == Vars::Chat::Room.Value)
                            {
                                std::string targetRoomId = room["room_id"];
                                QueueMessage("Matrix Debug", "Found target room in hierarchy: " + targetRoomId);
                                
                                // Join the specific room
                                auto roomJoinResponse = HttpClient::Post(
                                    m_sBaseUrl + "/_matrix/client/v3/rooms/" + targetRoomId + "/join",
                                    "{}",
                                    headers
                                );
                                
                                if (roomJoinResponse.status_code == 200)
                                {
                                    m_sRoomId = targetRoomId;
                                    QueueMessage("Matrix Debug", "Successfully joined target room: " + targetRoomId);
                                    return true;
                                }
                                else
                                {
                                    QueueMessage("Matrix Debug", "Failed to join target room: " + std::to_string(roomJoinResponse.status_code));
                                }
                            }
                        }
                    }
                    else
                    {
                        QueueMessage("Matrix Debug", "No rooms found in hierarchy");
                    }
                }
                else
                {
                    QueueMessage("Matrix Debug", "Failed to get space hierarchy: " + hierarchyResponse.text);
                    QueueMessage("Matrix Debug", "Trying to join room directly by alias...");
                    
                    // Try to join the room directly by alias since hierarchy failed
                    std::string roomAlias = "#" + Vars::Chat::Room.Value + ":" + Vars::Chat::Server.Value;
                    std::string encodedRoomAlias = HttpClient::UrlEncode(roomAlias);
                    std::string roomDirectoryUrl = m_sBaseUrl + "/_matrix/client/v3/directory/room/" + encodedRoomAlias;
                    
                    auto roomDirectoryResponse = HttpClient::Get(roomDirectoryUrl, headers);
                    
                    if (roomDirectoryResponse.status_code == 200)
                    {
                        auto roomDirectoryJson = nlohmann::json::parse(roomDirectoryResponse.text);
                        std::string targetRoomId = roomDirectoryJson["room_id"];
                        QueueMessage("Matrix Debug", "Found room " + roomAlias + " -> " + targetRoomId);
                        
                        // Try to join it
                        auto roomJoinResponse = HttpClient::Post(
                            m_sBaseUrl + "/_matrix/client/v3/rooms/" + targetRoomId + "/join",
                            "{}",
                            headers
                        );
                        
                        if (roomJoinResponse.status_code == 200)
                        {
                            m_sRoomId = targetRoomId;
                            QueueMessage("Matrix Debug", "Successfully joined room directly: " + targetRoomId);
                            return true;
                        }
                        else
                        {
                            QueueMessage("Matrix Debug", "Failed to join room " + roomAlias + ": " + std::to_string(roomJoinResponse.status_code));
                        }
                    }
                    else
                    {
                        QueueMessage("Matrix Debug", "Room " + roomAlias + " not found in directory");
                    }
                }
                
                // Use the space itself for messaging if no specific room found
                m_sRoomId = spaceRoomId;
                QueueMessage("Matrix Debug", "Using space as messaging room: " + spaceRoomId);
                return true;
            }
        }
        
        // Strategy 2: Try direct room lookup and join
        std::string roomAlias = "#" + Vars::Chat::Room.Value + ":" + Vars::Chat::Server.Value;
        std::string encodedRoomAlias = HttpClient::UrlEncode(roomAlias);
        std::string roomDirectoryUrl = m_sBaseUrl + "/_matrix/client/v3/directory/room/" + encodedRoomAlias;
        
        QueueMessage("Matrix Debug", "Looking up room directly: " + roomAlias);
        QueueMessage("Matrix Debug", "Encoded room alias: " + encodedRoomAlias);
        QueueMessage("Matrix Debug", "Room directory URL: " + roomDirectoryUrl);
        
        auto roomDirectoryResponse = HttpClient::Get(roomDirectoryUrl, headers);
        
        QueueMessage("Matrix Debug", "Room directory response code: " + std::to_string(roomDirectoryResponse.status_code));
        QueueMessage("Matrix Debug", "Room directory response: " + roomDirectoryResponse.text);
        
        if (roomDirectoryResponse.status_code == 200)
        {
            auto roomDirectoryJson = nlohmann::json::parse(roomDirectoryResponse.text);
            std::string targetRoomId = roomDirectoryJson["room_id"];
            QueueMessage("Matrix Debug", "Found room in directory: " + targetRoomId);
            
            // Join the room using room ID
            auto joinRoomResponse = HttpClient::Post(
                m_sBaseUrl + "/_matrix/client/v3/rooms/" + targetRoomId + "/join",
                "{}",
                headers
            );
            
            if (joinRoomResponse.status_code == 200)
            {
                m_sRoomId = targetRoomId;
                QueueMessage("Matrix Debug", "Successfully joined room directly: " + targetRoomId);
                return true;
            }
        }
        
        // Strategy 3: Try direct join using alias (bypass directory)
        QueueMessage("Matrix Debug", "Trying direct join with alias: " + spaceAlias);
        
        auto directJoinSpaceResponse = HttpClient::Post(
            m_sBaseUrl + "/_matrix/client/v3/join/" + HttpClient::UrlEncode(spaceAlias),
            "{}",
            headers
        );
        
        QueueMessage("Matrix Debug", "Direct space join response: " + std::to_string(directJoinSpaceResponse.status_code));
        QueueMessage("Matrix Debug", "Direct space join response body: " + directJoinSpaceResponse.text);
        
        if (directJoinSpaceResponse.status_code == 200)
        {
            auto spaceJson = nlohmann::json::parse(directJoinSpaceResponse.text);
            m_sRoomId = spaceJson["room_id"];
            QueueMessage("Matrix Debug", "Successfully joined space via direct join: " + m_sRoomId);
            return true;
        }
        
        QueueMessage("Matrix Debug", "Trying direct join with room alias: " + roomAlias);
        
        auto directJoinRoomResponse = HttpClient::Post(
            m_sBaseUrl + "/_matrix/client/v3/join/" + HttpClient::UrlEncode(roomAlias),
            "{}",
            headers
        );
        
        QueueMessage("Matrix Debug", "Direct room join response: " + std::to_string(directJoinRoomResponse.status_code));
        QueueMessage("Matrix Debug", "Direct room join response body: " + directJoinRoomResponse.text);
        
        if (directJoinRoomResponse.status_code == 200)
        {
            auto roomJson = nlohmann::json::parse(directJoinRoomResponse.text);
            m_sRoomId = roomJson["room_id"];
            QueueMessage("Matrix Debug", "Successfully joined room via direct join: " + m_sRoomId);
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
            QueueMessage("Matrix Debug", "Trying alternative alias: " + alias);
            
            auto altJoinResponse = HttpClient::Post(
                m_sBaseUrl + "/_matrix/client/v3/join/" + HttpClient::UrlEncode(alias),
                "{}",
                headers
            );
            
            if (altJoinResponse.status_code == 200)
            {
                auto altJson = nlohmann::json::parse(altJoinResponse.text);
                m_sRoomId = altJson["room_id"];
                QueueMessage("Matrix Debug", "Successfully joined with alternative alias: " + alias);
                return true;
            }
            else
            {
                QueueMessage("Matrix Debug", "Alternative alias failed (" + std::to_string(altJoinResponse.status_code) + "): " + altJoinResponse.text);
            }
        }
        
        // Strategy 4: Check what public rooms are available
        QueueMessage("Matrix Debug", "Checking public rooms directory...");
        
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
                    QueueMessage("Matrix Debug", "Found " + std::to_string(publicJson["chunk"].size()) + " public rooms:");
                    for (const auto& room : publicJson["chunk"])
                    {
                        if (room.contains("name") && room.contains("room_id"))
                        {
                            std::string roomName = room["name"];
                            std::string roomId = room["room_id"];
                            QueueMessage("Matrix Debug", "- " + roomName + " (" + roomId + ")");
                            
                            // If we find any room that looks like it might be ours, try to join it
                            if (roomName.find("talk") != std::string::npos || 
                                roomName.find("async") != std::string::npos ||
                                roomName.find("amalgam") != std::string::npos)
                            {
                                QueueMessage("Matrix Debug", "Attempting to join promising room: " + roomName);
                                
                                auto joinPublicResponse = HttpClient::Post(
                                    m_sBaseUrl + "/_matrix/client/v3/rooms/" + roomId + "/join",
                                    "{}",
                                    headers
                                );
                                
                                if (joinPublicResponse.status_code == 200)
                                {
                                    m_sRoomId = roomId;
                                    QueueMessage("Matrix Debug", "Successfully joined public room: " + roomName);
                                    return true;
                                }
                            }
                        }
                    }
                }
            } catch (...) {
                QueueMessage("Matrix Debug", "Error parsing public rooms response");
            }
        }
        
        // Strategy 4.5: List all joined rooms and look for matches
        QueueMessage("Matrix Debug", "Checking existing joined rooms...");
        
        auto syncResponse = HttpClient::Get(
            m_sBaseUrl + "/_matrix/client/v3/sync?timeout=0",
            headers
        );
        
        if (syncResponse.status_code == 200)
        {
            auto syncJson = nlohmann::json::parse(syncResponse.text);
            
            if (syncJson.contains("rooms") && syncJson["rooms"].contains("join"))
            {
                QueueMessage("Matrix Debug", "Found " + std::to_string(syncJson["rooms"]["join"].size()) + " joined rooms");
                
                for (const auto& [roomId, roomData] : syncJson["rooms"]["join"].items())
                {
                    QueueMessage("Matrix Debug", "Checking room: " + roomId);
                    
                    // Simple check - just use the first room we find
                    if (syncJson["rooms"]["join"].size() > 0)
                    {
                        m_sRoomId = roomId;
                        QueueMessage("Matrix Debug", "Using existing room: " + roomId);
                        return true;
                    }
                }
            }
        }
        
        // Strategy 5: Create the room/space if they don't exist
        QueueMessage("Matrix Debug", "Creating asyncroom space since it doesn't exist...");
        
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
            
            QueueMessage("Matrix Debug", "Successfully created space: " + spaceId);
            
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
                
                QueueMessage("Matrix Debug", "Successfully created talk room: " + m_sRoomId);
                
                // Add room to space hierarchy
                nlohmann::json spaceChildData = {
                    {"via", {Vars::Chat::Server.Value}}
                };
                
                auto addToSpaceResponse = HttpClient::Put(
                    m_sBaseUrl + "/_matrix/client/v3/rooms/" + spaceId + "/state/m.space.child/" + m_sRoomId,
                    spaceChildData.dump(),
                    headers
                );
                
                QueueMessage("Matrix Debug", "Added room to space hierarchy");
                return true;
            }
            else
            {
                // Use the space itself for messaging
                m_sRoomId = spaceId;
                QueueMessage("Matrix Debug", "Using space for messaging: " + spaceId);
                return true;
            }
        }
        
        // Strategy 6: Just create a simple public room called "amalgam"
        QueueMessage("Matrix Debug", "Creating fallback room: amalgam");
        
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
            QueueMessage("Matrix Debug", "Successfully created fallback room: " + m_sRoomId);
            return true;
        }
        
        QueueMessage("Matrix Debug", "All strategies failed including room creation");
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
        QueueMessage("Matrix Debug", "Attempting to send message to room: " + m_sRoomId);
        
        std::string eventType = "m.room.message";
        std::string content;
        
        // Prepare message content
        nlohmann::json msgData = {
            {"msgtype", "m.text"},
            {"body", message}
        };
        
        // Check if encryption is enabled
        if (IsEncryptionEnabled()) {
            QueueMessage("Matrix Debug", "Encrypting message with Megolm");
            
            // Share session key with other users in the room
            // TODO: Get actual user list from room members - for now sharing with common test users
            std::vector<std::string> user_ids = {
                Vars::Chat::Username.Value + ":" + Vars::Chat::Server.Value,  // ourselves
                "@async_123:" + Vars::Chat::Server.Value,   // other test users
                "@i56i56ii56yu45:" + Vars::Chat::Server.Value,
                "@y345y453yy:" + Vars::Chat::Server.Value,
                "@y4u57yhh:" + Vars::Chat::Server.Value
            };
            
            // Claim one-time keys for proper Olm session establishment
            HttpClaimKeys(user_ids);
            
            // Share the session key with the users
            m_pCrypto->ShareSessionKey(m_sRoomId, user_ids);
            
            // Send any pending room keys via to-device messages
            auto pendingKeys = m_pCrypto->GetPendingRoomKeys();
            for (const auto& key : pendingKeys) {
                HttpSendToDevice("m.room.encrypted", key.user_id, key.device_id, key.encrypted_content);
            }
            m_pCrypto->ClearPendingRoomKeys();
            
            // Encrypt the message content
            std::string encryptedContent = m_pCrypto->EncryptMessage(m_sRoomId, eventType, msgData.dump());
            if (!encryptedContent.empty()) {
                eventType = "m.room.encrypted";
                content = encryptedContent;
                QueueMessage("Matrix Debug", "Message encrypted successfully");
            } else {
                QueueMessage("Matrix Debug", "Encryption failed, sending unencrypted");
                content = msgData.dump();
            }
        }
        
        std::string txnId = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
        
        std::map<std::string, std::string> headers = {
            {"Authorization", "Bearer " + m_sAccessToken},
            {"Content-Type", "application/json"}
        };
        
        std::string url = m_sBaseUrl + "/_matrix/client/v3/rooms/" + m_sRoomId + "/send/" + eventType + "/" + txnId;
        QueueMessage("Matrix Debug", "Send URL: " + url);
        
        auto response = HttpClient::Put(url, content, headers);
        
        if (response.status_code == 200)
        {
            QueueMessage("Matrix Debug", "Message sent successfully!");
            
            // Show our own message in console immediately
            std::string ourUserId = Vars::Chat::Username.Value + ":" + Vars::Chat::Server.Value;
            ProcessIncomingMessage(ourUserId, message);
            
            return true;
        }
        else
        {
            QueueMessage("Matrix Debug", "Send failed with status: " + std::to_string(response.status_code));
            std::lock_guard<std::mutex> lock(m_StatusMutex);
            m_sLastError = "Send message failed with status: " + std::to_string(response.status_code);
            return false;
        }
    }
    catch (const std::exception& e)
    {
        std::lock_guard<std::mutex> lock(m_StatusMutex);
        m_sLastError = "Send message error: " + std::string(e.what());
        QueueMessage("Matrix Debug", "Send exception: " + std::string(e.what()));
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
                QueueMessage("Matrix Debug", "First sync successful, monitoring for messages...");
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
                    QueueMessage("Matrix Debug", "Rooms we're joined to:");
                    for (const auto& [roomId, roomData] : json["rooms"]["join"].items())
                    {
                        QueueMessage("Matrix Debug", "- Room ID: " + roomId);
                    }
                    QueueMessage("Matrix Debug", "Looking for messages in: " + m_sRoomId);
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
                                QueueMessage("Matrix Debug", "Encrypted message received from " + sender);
                                if (event.contains("content")) {
                                    content = m_pCrypto->DecryptMessage(roomId, event["content"].dump());
                                    if (!content.empty() && !content.starts_with("[")) {
                                        QueueMessage("Matrix Debug", "Decrypted message: " + content);
                                        processed = true;
                                    } else {
                                        QueueMessage("Matrix Debug", "Decryption failed: " + content);
                                    }
                                } else {
                                    QueueMessage("Matrix Debug", "Encrypted event missing content");
                                }
                            }
                            // Handle unencrypted text messages
                            else if (event.contains("type") && event["type"] == "m.room.message" && 
                                     event.contains("content") && event["content"].contains("msgtype") && 
                                     event["content"]["msgtype"] == "m.text")
                            {
                                content = event["content"]["body"];
                                QueueMessage("Matrix Debug", "Unencrypted message from " + sender + ": " + content);
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
                        QueueMessage("Matrix Debug", "Processed room key event");
                    }
                    else if (event.contains("type") && event["type"] == "m.room.encrypted")
                    {
                        // Decrypt and process encrypted to-device messages
                        // This would contain room keys encrypted with Olm
                        QueueMessage("Matrix Debug", "Received encrypted to-device message");
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
                QueueMessage("Matrix Debug", "Sync HTTP error " + std::to_string(response.status_code) + ": " + response.text);
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
            QueueMessage("Matrix Debug", "Sync exception: " + std::string(e.what()));
            errorCount++;
        }
    }
}

bool CChat::HttpUploadDeviceKeys()
{
    if (!m_pCrypto) {
        QueueMessage("Matrix Debug", "Cannot upload device keys - encryption not initialized");
        return false;
    }
    
    try {
        std::string deviceKeysJson = m_pCrypto->GetDeviceKeys();
        if (deviceKeysJson.empty()) {
            QueueMessage("Matrix Debug", "Failed to get device keys");
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
            QueueMessage("Matrix Debug", "Device keys uploaded successfully");
            return true;
        } else {
            QueueMessage("Matrix Debug", "Failed to upload device keys: " + std::to_string(response.status_code) + " - " + response.text);
            return false;
        }
    } catch (const std::exception& e) {
        QueueMessage("Matrix Debug", "Device key upload error: " + std::string(e.what()));
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
            // Get users in the current room from last sync
            // For now, we'll just query our own user_id as a fallback
            if (m_pCrypto) {
                std::string our_user_id = m_pCrypto->GetDeviceId(); // This should be user_id, need to fix
                // TODO: Store user_id properly in crypto class
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
                QueueMessage("Matrix Debug", "Downloaded device keys for " + std::to_string(responseJson["device_keys"].size()) + " users");
                return true;
            }
        } else {
            QueueMessage("Matrix Debug", "Failed to download device keys: " + std::to_string(response.status_code));
        }
        
        return false;
    } catch (const std::exception& e) {
        QueueMessage("Matrix Debug", "Device key download error: " + std::string(e.what()));
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
            QueueMessage("Matrix Debug", "To-device message sent: " + event_type);
            return true;
        } else {
            QueueMessage("Matrix Debug", "Failed to send to-device message: " + std::to_string(response.status_code));
            return false;
        }
    } catch (const std::exception& e) {
        QueueMessage("Matrix Debug", "To-device send error: " + std::string(e.what()));
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
            QueueMessage("Matrix Debug", "No one-time keys to upload");
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
            QueueMessage("Matrix Debug", "One-time keys uploaded successfully");
            return true;
        } else {
            QueueMessage("Matrix Debug", "Failed to upload one-time keys: " + std::to_string(response.status_code));
            return false;
        }
    } catch (const std::exception& e) {
        QueueMessage("Matrix Debug", "One-time key upload error: " + std::string(e.what()));
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
                QueueMessage("Matrix Debug", "Found " + std::to_string(responseJson["chunk"].size()) + " room members");
                // TODO: Store room members for key sharing
                return true;
            }
        } else {
            QueueMessage("Matrix Debug", "Failed to get room members: " + std::to_string(response.status_code));
        }
        
        return false;
    } catch (const std::exception& e) {
        QueueMessage("Matrix Debug", "Room members error: " + std::string(e.what()));
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
                QueueMessage("Matrix Debug", "Claimed one-time keys for " + std::to_string(responseJson["one_time_keys"].size()) + " users");
                
                // Process claimed keys and store them for session creation
                for (auto& [userId, devices] : responseJson["one_time_keys"].items()) {
                    for (auto& [deviceId, keys] : devices.items()) {
                        for (auto& [keyId, keyValue] : keys.items()) {
                            // Find the device's curve25519 key to map to this one-time key
                            // For now, we'll use a simplified mapping
                            std::string device_curve_key = "device_" + userId + "_" + deviceId;
                            m_pCrypto->StoreClaimedKey(device_curve_key, keyValue);
                            QueueMessage("Matrix Debug", "Stored claimed key " + keyId + " from " + userId + ":" + deviceId);
                        }
                    }
                }
                return true;
            }
        } else {
            QueueMessage("Matrix Debug", "Failed to claim keys: " + std::to_string(response.status_code));
        }
        
        return false;
    } catch (const std::exception& e) {
        QueueMessage("Matrix Debug", "Key claim error: " + std::string(e.what()));
        return false;
    }
}

bool CChat::InitializeEncryption()
{
    if (!m_pCrypto) {
        return false;
    }
    
    // Initialize crypto with our user ID
    bool success = m_pCrypto->Initialize(Vars::Chat::Username.Value + ":" + Vars::Chat::Server.Value);
    if (success) {
        m_bEncryptionEnabled = true;
        QueueMessage("Matrix", "Encryption initialized for device: " + m_pCrypto->GetDeviceId());
        
        // Upload our device keys to the server
        if (HttpUploadDeviceKeys()) {
            QueueMessage("Matrix Debug", "Device keys uploaded successfully");
        } else {
            QueueMessage("Matrix Debug", "Failed to upload device keys");
        }
        
        // Upload one-time keys for session establishment
        if (HttpUploadOneTimeKeys()) {
            QueueMessage("Matrix Debug", "One-time keys uploaded successfully");
        } else {
            QueueMessage("Matrix Debug", "Failed to upload one-time keys");
        }
        
        // Download device keys for other users in the room
        HttpDownloadDeviceKeys();
        
        // Get room members for key sharing
        HttpGetRoomMembers();
        
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
        chatSettings["server"] = Vars::Chat::Server.Value;
        chatSettings["username"] = Vars::Chat::Username.Value;
        chatSettings["password"] = Vars::Chat::Password.Value;
        chatSettings["space"] = Vars::Chat::Space.Value;
        chatSettings["room"] = Vars::Chat::Room.Value;
        chatSettings["auto_connect"] = Vars::Chat::AutoConnect.Value;
        chatSettings["show_timestamps"] = Vars::Chat::ShowTimestamps.Value;
        
        std::ofstream file("chat_settings.json");
        if (file.is_open())
        {
            file << chatSettings.dump(4);
            file.close();
        }
    }
    catch (const std::exception& e)
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
            
            if (chatSettings.contains("server"))
                Vars::Chat::Server.Value = chatSettings["server"].get<std::string>();
            if (chatSettings.contains("username"))
                Vars::Chat::Username.Value = chatSettings["username"].get<std::string>();
            if (chatSettings.contains("password"))
                Vars::Chat::Password.Value = chatSettings["password"].get<std::string>();
            if (chatSettings.contains("space"))
                Vars::Chat::Space.Value = chatSettings["space"].get<std::string>();
            if (chatSettings.contains("room"))
                Vars::Chat::Room.Value = chatSettings["room"].get<std::string>();
            if (chatSettings.contains("auto_connect"))
                Vars::Chat::AutoConnect.Value = chatSettings["auto_connect"].get<bool>();
            if (chatSettings.contains("show_timestamps"))
                Vars::Chat::ShowTimestamps.Value = chatSettings["show_timestamps"].get<bool>();
        }
    }
    catch (const std::exception& e)
    {
        // Silently ignore load errors - use defaults
    }
}