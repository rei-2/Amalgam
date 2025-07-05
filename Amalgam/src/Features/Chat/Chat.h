#pragma once

// Include system headers first
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <queue>

// Include SDK after standard headers
#include "../../SDK/SDK.h"
#include <ImGui/imgui.h>

// Include our isolated HTTP client
#include "HttpClient.h"
#include "MatrixCrypto.h"

// JSON parsing (header-only, safer to include)
#include <nlohmann/json.hpp>

// Message queue structure for thread-safe UI updates
struct ChatMessage
{
    std::string sender;
    std::string content;
};

class CChat
{
private:
    std::thread m_ClientThread;
    std::atomic<bool> m_bConnected{false};
    std::atomic<bool> m_bLoginInProgress{false};
    std::atomic<bool> m_bShouldStop{false};
    std::string m_sLastError;
    std::string m_sLastSuccess;
    mutable std::mutex m_StatusMutex;
    
    // Thread-safe message queue for UI updates
    std::queue<ChatMessage> m_MessageQueue;
    mutable std::mutex m_MessageQueueMutex;
    
    // Matrix client data
    std::string m_sAccessToken;
    std::string m_sRoomId;
    std::string m_sBaseUrl;
    std::string m_sNextBatch;
    
    // Encryption support
    std::unique_ptr<MatrixCrypto> m_pCrypto;
    bool m_bEncryptionEnabled = false;
    
    // Helper functions
    void ProcessIncomingMessage(const std::string& sender, const std::string& content);
    void HandleMatrixEvents();
    void DisplayInGameMessage(const std::string& sender, const std::string& content);
    void QueueMessage(const std::string& sender, const std::string& content);
    
    // Chat settings persistence (separate from main config system)
    void LoadChatSettings();
    
    // Matrix HTTP API calls
    bool HttpLogin(const std::string& username, const std::string& password);
    bool HttpJoinRoom();
    bool HttpSendMessage(const std::string& message);
    void HttpSync();
    
public:
    CChat();
    ~CChat();
    
    // Connection management
    void Connect();
    void Disconnect();
    void CreateAccount(const std::string& username, const std::string& password);
    void Login(const std::string& username, const std::string& password);
    
    // Chat settings persistence
    void SaveChatSettings();
    
    // Message handling
    void SendMessage(const std::string& message);
    
    // Encryption
    bool InitializeEncryption();
    void EnableEncryption(bool enable) { m_bEncryptionEnabled = enable; }
    bool IsEncryptionEnabled() const { return m_bEncryptionEnabled && m_pCrypto != nullptr; }
    
    // Status
    bool IsConnected() const { return m_bConnected.load(); }
    bool IsLoginInProgress() const { return m_bLoginInProgress.load(); }
    std::string GetLastError() const { 
        std::lock_guard<std::mutex> lock(m_StatusMutex);
        return m_sLastError; 
    }
    std::string GetLastSuccess() const { 
        std::lock_guard<std::mutex> lock(m_StatusMutex);
        return m_sLastSuccess; 
    }
    
    // Game chat integration
    bool ProcessGameChatMessage(const std::string& message);
    
    // UI rendering
    void DrawConnectionStatus();
    
    // Process queued messages from main thread
    void ProcessQueuedMessages();
};

ADD_FEATURE(CChat, Chat)