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
        std::lock_guard<std::mutex> lock(m_ErrorMutex);
        m_sLastError = "Please configure server, username, and password first.";
        return;
    }
    
    m_bLoginInProgress.store(true);
    {
        std::lock_guard<std::mutex> lock(m_ErrorMutex);
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
        DisplayInGameMessage("Matrix Debug", "Testing connectivity to: " + m_sBaseUrl);
        auto versionResponse = HttpClient::Get(m_sBaseUrl + "/_matrix/client/versions", {});
        DisplayInGameMessage("Matrix Debug", "Server versions check status: " + std::to_string(versionResponse.status_code));
        
        if (versionResponse.status_code == 0)
        {
            DisplayInGameMessage("Matrix Debug", "Network connectivity failed: " + versionResponse.text);
            std::lock_guard<std::mutex> lock(m_ErrorMutex);
            m_sLastError = "Cannot connect to server: " + versionResponse.text;
            m_bLoginInProgress.store(false);
            return;
        }
        
        // Try login first
        if (HttpLogin(username, password) && HttpJoinRoom())
        {
            m_bLoginInProgress.store(false);
            m_bConnected.store(true);
            
            DisplayInGameMessage("Matrix", "Connected to room '" + Vars::Chat::Room.Value + "' in space '" + Vars::Chat::Space.Value + "'");
            DisplayInGameMessage("Matrix", "Use !! prefix to send messages to Matrix chat");
            
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
                std::lock_guard<std::mutex> lock(m_ErrorMutex);
                shouldCreateAccount = (m_sLastError.find("Unknown user") != std::string::npos ||
                                     m_sLastError.find("M_USER_DEACTIVATED") != std::string::npos ||
                                     m_sLastError.find("M_UNKNOWN") != std::string::npos ||
                                     m_sLastError.find("not found") != std::string::npos);
            }
            
            if (shouldCreateAccount)
            {
                // User doesn't exist, try to create account
                DisplayInGameMessage("Matrix", "User not found, attempting to create account...");
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
                std::lock_guard<std::mutex> lock(m_ErrorMutex);
                m_sLastError = "Account created successfully! You are now logged in.";
            }
            
            // Join room and start sync (outside of mutex lock)
            if (HttpJoinRoom())
            {
                DisplayInGameMessage("Matrix", "Account created and connected to room '" + Vars::Chat::Room.Value + "' in space '" + Vars::Chat::Space.Value + "'");
                DisplayInGameMessage("Matrix", "Use !! prefix to send messages to Matrix chat");
                
                if (!m_ClientThread.joinable())
                {
                    m_ClientThread = std::thread(&CChat::HandleMatrixEvents, this);
                }
            }
        }
        else if (response.status_code == 400)
        {
            auto json = nlohmann::json::parse(response.text);
            std::lock_guard<std::mutex> lock(m_ErrorMutex);
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
            std::lock_guard<std::mutex> lock(m_ErrorMutex);
            m_sLastError = "Registration disabled on this server. Contact server admin.";
        }
        else
        {
            auto json = nlohmann::json::parse(response.text);
            std::lock_guard<std::mutex> lock(m_ErrorMutex);
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
        
        std::lock_guard<std::mutex> lock(m_ErrorMutex);
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
    DisplayInGameMessage("Matrix Debug", "Processing message: '" + message + "'");
    
    // Check if message starts with !! prefix
    if (message.length() >= 2 && message.substr(0, 2) == "!!")
    {
        DisplayInGameMessage("Matrix Debug", "Found !! prefix, processing Matrix message");
        
        std::string chatMessage = message.substr(2); // Remove !! prefix
        
        // Trim whitespace
        while (!chatMessage.empty() && (chatMessage.front() == ' ' || chatMessage.front() == '\t'))
            chatMessage.erase(0, 1);
        
        if (!chatMessage.empty())
        {
            DisplayInGameMessage("Matrix Debug", "Sending: " + chatMessage);
            
            // Check if we're connected before sending
            if (!m_bConnected.load())
            {
                DisplayInGameMessage("Matrix", "Not connected to Matrix server");
                return true; // Still block from game chat
            }
            
            SendMessage(chatMessage);
            DisplayInGameMessage("Matrix Debug", "Blocking message from game chat");
            return true; // Message was processed and should be blocked from game chat
        }
        else
        {
            DisplayInGameMessage("Matrix Debug", "Empty message after trimming, blocking from game chat");
            return true; // Block empty !! messages from game chat
        }
    }
    
    DisplayInGameMessage("Matrix Debug", "No !! prefix, allowing normal chat");
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
        DisplayInGameMessage("[Matrix] " + senderName, content);
    }
}

void CChat::DisplayInGameMessage(const std::string& sender, const std::string& content)
{
    // Always print to console/stdout as well
    printf("[Matrix] %s: %s\n", sender.c_str(), content.c_str());
    
    // Print to TF2's console for guaranteed visibility
    if (I::CVar)
    {
        I::CVar->ConsolePrintf("[Matrix] %s: %s\n", sender.c_str(), content.c_str());
    }
    
    // Only show non-debug messages in in-game chat
    if (sender.find("Debug") == std::string::npos)
    {
        // Display in TF2's in-game chat
        if (I::ClientModeShared && I::ClientModeShared->m_pChatElement)
        {
            // Use TF2's proper color system with hex codes
            std::string cyan_color = std::string("\x7") + "00FFFF";  // Cyan hex color
            std::string green_color = std::string("\x7") + "00FF00"; // Green hex color  
            std::string white_color = std::string("\x1");            // Normal white color
            
            if (Vars::Chat::ShowTimestamps.Value)
            {
                auto now = std::chrono::system_clock::now();
                auto time_t = std::chrono::system_clock::to_time_t(now);
                auto tm = *std::localtime(&time_t);
                
                char timeStr[16];
                strftime(timeStr, sizeof(timeStr), "%H:%M", &tm);
                
                I::ClientModeShared->m_pChatElement->ChatPrintf(0, "%s[Matrix %s]%s %s%s%s: %s", 
                    cyan_color.c_str(), timeStr, white_color.c_str(), 
                    green_color.c_str(), sender.c_str(), white_color.c_str(), 
                    content.c_str());
            }
            else
            {
                I::ClientModeShared->m_pChatElement->ChatPrintf(0, "%s[Matrix]%s %s%s%s: %s", 
                    cyan_color.c_str(), white_color.c_str(), 
                    green_color.c_str(), sender.c_str(), white_color.c_str(), 
                    content.c_str());
            }
        }
    }
}

void CChat::HandleMatrixEvents()
{
    while (m_bConnected.load() && !m_bShouldStop.load())
    {
        HttpSync();
        
        // Sleep with interruption check for faster shutdown
        for (int i = 0; i < 50 && !m_bShouldStop.load(); ++i)
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
    
    std::string errorText;
    {
        std::lock_guard<std::mutex> lock(m_ErrorMutex);
        errorText = m_sLastError;
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
            DisplayInGameMessage("Matrix Debug", "Login successful, access token obtained");
            return true;
        }
        else
        {
            std::lock_guard<std::mutex> lock(m_ErrorMutex);
            if (response.status_code == 0)
            {
                m_sLastError = "Network connection failed during login: " + response.text;
                DisplayInGameMessage("Matrix Debug", "Login failed with network error (status 0)");
                DisplayInGameMessage("Matrix Debug", "Error details: " + response.text);
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
        std::lock_guard<std::mutex> lock(m_ErrorMutex);
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
        
        DisplayInGameMessage("Matrix Debug", "Starting room join process...");
        DisplayInGameMessage("Matrix Debug", "Space: " + Vars::Chat::Space.Value + ", Room: " + Vars::Chat::Room.Value + ", Server: " + Vars::Chat::Server.Value);
        DisplayInGameMessage("Matrix Debug", "Base URL: " + m_sBaseUrl);
        
        // Test basic HTTP connectivity first
        DisplayInGameMessage("Matrix Debug", "Testing HTTP connectivity...");
        auto testResponse = HttpClient::Get(m_sBaseUrl + "/_matrix/client/versions", {});
        DisplayInGameMessage("Matrix Debug", "Connectivity test result: " + std::to_string(testResponse.status_code));
        DisplayInGameMessage("Matrix Debug", "Connectivity test response: " + testResponse.text);
        
        // Strategy 1: Try to resolve and join the space first using directory lookup
        std::string spaceAlias = "#" + Vars::Chat::Space.Value + ":" + Vars::Chat::Server.Value;
        std::string encodedSpaceAlias = HttpClient::UrlEncode(spaceAlias);
        std::string spaceDirectoryUrl = m_sBaseUrl + "/_matrix/client/v3/directory/room/" + encodedSpaceAlias;
        
        DisplayInGameMessage("Matrix Debug", "Looking up space in directory: " + spaceAlias);
        DisplayInGameMessage("Matrix Debug", "Encoded alias: " + encodedSpaceAlias);
        DisplayInGameMessage("Matrix Debug", "Full URL: " + spaceDirectoryUrl);
        
        auto spaceDirectoryResponse = HttpClient::Get(spaceDirectoryUrl, headers);
        
        DisplayInGameMessage("Matrix Debug", "Space directory response code: " + std::to_string(spaceDirectoryResponse.status_code));
        DisplayInGameMessage("Matrix Debug", "Space directory response: " + spaceDirectoryResponse.text);
        
        if (spaceDirectoryResponse.status_code == 200)
        {
            auto spaceDirectoryJson = nlohmann::json::parse(spaceDirectoryResponse.text);
            std::string spaceRoomId = spaceDirectoryJson["room_id"];
            DisplayInGameMessage("Matrix Debug", "Found space in directory: " + spaceRoomId);
            
            // Join the space using room ID
            auto joinSpaceResponse = HttpClient::Post(
                m_sBaseUrl + "/_matrix/client/v3/rooms/" + spaceRoomId + "/join",
                "{}",
                headers
            );
            
            if (joinSpaceResponse.status_code == 200)
            {
                DisplayInGameMessage("Matrix Debug", "Successfully joined space: " + spaceRoomId);
                
                // Look for the talk room within the space hierarchy
                auto hierarchyResponse = HttpClient::Get(
                    m_sBaseUrl + "/_matrix/client/v3/rooms/" + spaceRoomId + "/hierarchy",
                    headers
                );
                
                DisplayInGameMessage("Matrix Debug", "Space hierarchy response: " + std::to_string(hierarchyResponse.status_code));
                
                if (hierarchyResponse.status_code == 200)
                {
                    auto hierarchyJson = nlohmann::json::parse(hierarchyResponse.text);
                    DisplayInGameMessage("Matrix Debug", "Got space hierarchy, looking for room: " + Vars::Chat::Room.Value);
                    DisplayInGameMessage("Matrix Debug", "Hierarchy response: " + hierarchyResponse.text);
                    
                    if (hierarchyJson.contains("rooms"))
                    {
                        DisplayInGameMessage("Matrix Debug", "Found " + std::to_string(hierarchyJson["rooms"].size()) + " rooms in hierarchy");
                        
                        for (const auto& room : hierarchyJson["rooms"])
                        {
                            std::string roomName = room.contains("name") ? room["name"].get<std::string>() : "unnamed";
                            std::string roomId = room.contains("room_id") ? room["room_id"].get<std::string>() : "unknown";
                            
                            DisplayInGameMessage("Matrix Debug", "Hierarchy room: " + roomName + " (" + roomId + ")");
                            
                            if (room.contains("name") && room["name"] == Vars::Chat::Room.Value)
                            {
                                std::string targetRoomId = room["room_id"];
                                DisplayInGameMessage("Matrix Debug", "Found target room in hierarchy: " + targetRoomId);
                                
                                // Join the specific room
                                auto roomJoinResponse = HttpClient::Post(
                                    m_sBaseUrl + "/_matrix/client/v3/rooms/" + targetRoomId + "/join",
                                    "{}",
                                    headers
                                );
                                
                                if (roomJoinResponse.status_code == 200)
                                {
                                    m_sRoomId = targetRoomId;
                                    DisplayInGameMessage("Matrix Debug", "Successfully joined target room: " + targetRoomId);
                                    return true;
                                }
                                else
                                {
                                    DisplayInGameMessage("Matrix Debug", "Failed to join target room: " + std::to_string(roomJoinResponse.status_code));
                                }
                            }
                        }
                    }
                    else
                    {
                        DisplayInGameMessage("Matrix Debug", "No rooms found in hierarchy");
                    }
                }
                else
                {
                    DisplayInGameMessage("Matrix Debug", "Failed to get space hierarchy: " + hierarchyResponse.text);
                }
                
                // Use the space itself for messaging if no specific room found
                m_sRoomId = spaceRoomId;
                DisplayInGameMessage("Matrix Debug", "Using space as messaging room: " + spaceRoomId);
                return true;
            }
        }
        
        // Strategy 2: Try direct room lookup and join
        std::string roomAlias = "#" + Vars::Chat::Room.Value + ":" + Vars::Chat::Server.Value;
        std::string encodedRoomAlias = HttpClient::UrlEncode(roomAlias);
        std::string roomDirectoryUrl = m_sBaseUrl + "/_matrix/client/v3/directory/room/" + encodedRoomAlias;
        
        DisplayInGameMessage("Matrix Debug", "Looking up room directly: " + roomAlias);
        DisplayInGameMessage("Matrix Debug", "Encoded room alias: " + encodedRoomAlias);
        DisplayInGameMessage("Matrix Debug", "Room directory URL: " + roomDirectoryUrl);
        
        auto roomDirectoryResponse = HttpClient::Get(roomDirectoryUrl, headers);
        
        DisplayInGameMessage("Matrix Debug", "Room directory response code: " + std::to_string(roomDirectoryResponse.status_code));
        DisplayInGameMessage("Matrix Debug", "Room directory response: " + roomDirectoryResponse.text);
        
        if (roomDirectoryResponse.status_code == 200)
        {
            auto roomDirectoryJson = nlohmann::json::parse(roomDirectoryResponse.text);
            std::string targetRoomId = roomDirectoryJson["room_id"];
            DisplayInGameMessage("Matrix Debug", "Found room in directory: " + targetRoomId);
            
            // Join the room using room ID
            auto joinRoomResponse = HttpClient::Post(
                m_sBaseUrl + "/_matrix/client/v3/rooms/" + targetRoomId + "/join",
                "{}",
                headers
            );
            
            if (joinRoomResponse.status_code == 200)
            {
                m_sRoomId = targetRoomId;
                DisplayInGameMessage("Matrix Debug", "Successfully joined room directly: " + targetRoomId);
                return true;
            }
        }
        
        // Strategy 3: Try direct join using alias (bypass directory)
        DisplayInGameMessage("Matrix Debug", "Trying direct join with alias: " + spaceAlias);
        
        auto directJoinSpaceResponse = HttpClient::Post(
            m_sBaseUrl + "/_matrix/client/v3/join/" + HttpClient::UrlEncode(spaceAlias),
            "{}",
            headers
        );
        
        DisplayInGameMessage("Matrix Debug", "Direct space join response: " + std::to_string(directJoinSpaceResponse.status_code));
        DisplayInGameMessage("Matrix Debug", "Direct space join response body: " + directJoinSpaceResponse.text);
        
        if (directJoinSpaceResponse.status_code == 200)
        {
            auto spaceJson = nlohmann::json::parse(directJoinSpaceResponse.text);
            m_sRoomId = spaceJson["room_id"];
            DisplayInGameMessage("Matrix Debug", "Successfully joined space via direct join: " + m_sRoomId);
            return true;
        }
        
        DisplayInGameMessage("Matrix Debug", "Trying direct join with room alias: " + roomAlias);
        
        auto directJoinRoomResponse = HttpClient::Post(
            m_sBaseUrl + "/_matrix/client/v3/join/" + HttpClient::UrlEncode(roomAlias),
            "{}",
            headers
        );
        
        DisplayInGameMessage("Matrix Debug", "Direct room join response: " + std::to_string(directJoinRoomResponse.status_code));
        DisplayInGameMessage("Matrix Debug", "Direct room join response body: " + directJoinRoomResponse.text);
        
        if (directJoinRoomResponse.status_code == 200)
        {
            auto roomJson = nlohmann::json::parse(directJoinRoomResponse.text);
            m_sRoomId = roomJson["room_id"];
            DisplayInGameMessage("Matrix Debug", "Successfully joined room via direct join: " + m_sRoomId);
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
            DisplayInGameMessage("Matrix Debug", "Trying alternative alias: " + alias);
            
            auto altJoinResponse = HttpClient::Post(
                m_sBaseUrl + "/_matrix/client/v3/join/" + HttpClient::UrlEncode(alias),
                "{}",
                headers
            );
            
            if (altJoinResponse.status_code == 200)
            {
                auto altJson = nlohmann::json::parse(altJoinResponse.text);
                m_sRoomId = altJson["room_id"];
                DisplayInGameMessage("Matrix Debug", "Successfully joined with alternative alias: " + alias);
                return true;
            }
            else
            {
                DisplayInGameMessage("Matrix Debug", "Alternative alias failed (" + std::to_string(altJoinResponse.status_code) + "): " + altJoinResponse.text);
            }
        }
        
        // Strategy 4: Check what public rooms are available
        DisplayInGameMessage("Matrix Debug", "Checking public rooms directory...");
        
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
                    DisplayInGameMessage("Matrix Debug", "Found " + std::to_string(publicJson["chunk"].size()) + " public rooms:");
                    for (const auto& room : publicJson["chunk"])
                    {
                        if (room.contains("name") && room.contains("room_id"))
                        {
                            std::string roomName = room["name"];
                            std::string roomId = room["room_id"];
                            DisplayInGameMessage("Matrix Debug", "- " + roomName + " (" + roomId + ")");
                            
                            // If we find any room that looks like it might be ours, try to join it
                            if (roomName.find("talk") != std::string::npos || 
                                roomName.find("async") != std::string::npos ||
                                roomName.find("amalgam") != std::string::npos)
                            {
                                DisplayInGameMessage("Matrix Debug", "Attempting to join promising room: " + roomName);
                                
                                auto joinPublicResponse = HttpClient::Post(
                                    m_sBaseUrl + "/_matrix/client/v3/rooms/" + roomId + "/join",
                                    "{}",
                                    headers
                                );
                                
                                if (joinPublicResponse.status_code == 200)
                                {
                                    m_sRoomId = roomId;
                                    DisplayInGameMessage("Matrix Debug", "Successfully joined public room: " + roomName);
                                    return true;
                                }
                            }
                        }
                    }
                }
            } catch (...) {
                DisplayInGameMessage("Matrix Debug", "Error parsing public rooms response");
            }
        }
        
        // Strategy 4.5: List all joined rooms and look for matches
        DisplayInGameMessage("Matrix Debug", "Checking existing joined rooms...");
        
        auto syncResponse = HttpClient::Get(
            m_sBaseUrl + "/_matrix/client/v3/sync?timeout=0",
            headers
        );
        
        if (syncResponse.status_code == 200)
        {
            auto syncJson = nlohmann::json::parse(syncResponse.text);
            
            if (syncJson.contains("rooms") && syncJson["rooms"].contains("join"))
            {
                DisplayInGameMessage("Matrix Debug", "Found " + std::to_string(syncJson["rooms"]["join"].size()) + " joined rooms");
                
                for (const auto& [roomId, roomData] : syncJson["rooms"]["join"].items())
                {
                    DisplayInGameMessage("Matrix Debug", "Checking room: " + roomId);
                    
                    // Simple check - just use the first room we find
                    if (syncJson["rooms"]["join"].size() > 0)
                    {
                        m_sRoomId = roomId;
                        DisplayInGameMessage("Matrix Debug", "Using existing room: " + roomId);
                        return true;
                    }
                }
            }
        }
        
        // Strategy 5: Create the room/space if they don't exist
        DisplayInGameMessage("Matrix Debug", "Creating asyncroom space since it doesn't exist...");
        
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
            
            DisplayInGameMessage("Matrix Debug", "Successfully created space: " + spaceId);
            
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
                
                DisplayInGameMessage("Matrix Debug", "Successfully created talk room: " + m_sRoomId);
                
                // Add room to space hierarchy
                nlohmann::json spaceChildData = {
                    {"via", {Vars::Chat::Server.Value}}
                };
                
                auto addToSpaceResponse = HttpClient::Put(
                    m_sBaseUrl + "/_matrix/client/v3/rooms/" + spaceId + "/state/m.space.child/" + m_sRoomId,
                    spaceChildData.dump(),
                    headers
                );
                
                DisplayInGameMessage("Matrix Debug", "Added room to space hierarchy");
                return true;
            }
            else
            {
                // Use the space itself for messaging
                m_sRoomId = spaceId;
                DisplayInGameMessage("Matrix Debug", "Using space for messaging: " + spaceId);
                return true;
            }
        }
        
        // Strategy 6: Just create a simple public room called "amalgam"
        DisplayInGameMessage("Matrix Debug", "Creating fallback room: amalgam");
        
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
            DisplayInGameMessage("Matrix Debug", "Successfully created fallback room: " + m_sRoomId);
            return true;
        }
        
        DisplayInGameMessage("Matrix Debug", "All strategies failed including room creation");
        std::lock_guard<std::mutex> lock(m_ErrorMutex);
        m_sLastError = "Could not find, join, or create any rooms. Space: " + spaceAlias + ", Room: " + roomAlias;
        return false;
    }
    catch (const std::exception& e)
    {
        std::lock_guard<std::mutex> lock(m_ErrorMutex);
        m_sLastError = "Join room error: " + std::string(e.what());
        return false;
    }
}

bool CChat::HttpSendMessage(const std::string& message)
{
    try
    {
        DisplayInGameMessage("Matrix Debug", "Attempting to send message to room: " + m_sRoomId);
        
        nlohmann::json msgData = {
            {"msgtype", "m.text"},
            {"body", message}
        };
        
        std::string txnId = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
        
        std::map<std::string, std::string> headers = {
            {"Authorization", "Bearer " + m_sAccessToken},
            {"Content-Type", "application/json"}
        };
        
        std::string url = m_sBaseUrl + "/_matrix/client/v3/rooms/" + m_sRoomId + "/send/m.room.message/" + txnId;
        DisplayInGameMessage("Matrix Debug", "Send URL: " + url);
        
        auto response = HttpClient::Put(url, msgData.dump(), headers);
        
        if (response.status_code == 200)
        {
            DisplayInGameMessage("Matrix Debug", "Message sent successfully!");
            return true;
        }
        else
        {
            DisplayInGameMessage("Matrix Debug", "Send failed with status: " + std::to_string(response.status_code));
            std::lock_guard<std::mutex> lock(m_ErrorMutex);
            m_sLastError = "Send message failed with status: " + std::to_string(response.status_code);
            return false;
        }
    }
    catch (const std::exception& e)
    {
        std::lock_guard<std::mutex> lock(m_ErrorMutex);
        m_sLastError = "Send message error: " + std::string(e.what());
        DisplayInGameMessage("Matrix Debug", "Send exception: " + std::string(e.what()));
        return false;
    }
}

void CChat::HttpSync()
{
    try
    {
        std::string url = m_sBaseUrl + "/_matrix/client/v3/sync?timeout=30000";
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
                DisplayInGameMessage("Matrix Debug", "First sync successful, monitoring for messages...");
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
                    DisplayInGameMessage("Matrix Debug", "Rooms we're joined to:");
                    for (const auto& [roomId, roomData] : json["rooms"]["join"].items())
                    {
                        DisplayInGameMessage("Matrix Debug", "- Room ID: " + roomId);
                    }
                    DisplayInGameMessage("Matrix Debug", "Looking for messages in: " + m_sRoomId);
                    shownRooms = true;
                }
            }
            
            // Process events from ALL joined rooms
            if (json.contains("rooms") && json["rooms"].contains("join"))
            {
                for (const auto& [roomId, roomData] : json["rooms"]["join"].items())
                {
                    if (roomData.contains("timeline") && roomData["timeline"].contains("events"))
                    {
                        for (const auto& event : roomData["timeline"]["events"])
                        {
                            if (event["type"] == "m.room.message" && event["content"]["msgtype"] == "m.text")
                            {
                                std::string sender = event["sender"];
                                std::string content = event["content"]["body"];
                                
                                // Debug: Show message received with room info
                                DisplayInGameMessage("Matrix Debug", "Message in " + roomId + " from " + sender + ": " + content);
                                
                                ProcessIncomingMessage(sender, content);
                            }
                        }
                    }
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        // Log sync errors for debugging (but don't spam)
        static int errorCount = 0;
        if (errorCount < 5)
        {
            DisplayInGameMessage("Matrix Debug", "Sync error: " + std::string(e.what()));
            errorCount++;
        }
    }
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