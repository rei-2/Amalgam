#pragma once

// Include system headers first
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <queue>
#include <algorithm>

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
    std::atomic<bool> m_bAutoConnectAttempted{false};
    std::string m_sLastError;
    std::string m_sLastSuccess;
    mutable std::mutex m_StatusMutex;
    
    // Thread-safe message queue for UI updates
    std::queue<ChatMessage> m_MessageQueue;
    mutable std::mutex m_MessageQueueMutex;
    
    // Lazy initialization flag
    bool m_bInitialized;
    
    // Lazy initialization method
    void EnsureInitialized();
    
    // Matrix client data
    std::string m_sAccessToken;
    std::string m_sRoomId;
    std::string m_sBaseUrl;
    std::string m_sNextBatch;
    std::vector<std::string> m_vRoomMembers;
    
    // Email registration session data
    std::string m_sRegistrationSession;
    std::string m_sPendingUsername;
    std::string m_sPendingPassword;
    std::string m_sClientSecret;
    std::string m_sEmailSid;
    std::string m_sIdentityServer; // Track which identity server provided the SID
    std::string m_sRecaptchaPublicKey;
    bool m_bRecaptchaRequired = false;
    bool m_bRecaptchaShowing = false;
    
    // Email verification state tracking
    bool m_bEmailVerificationRequested = false;
    bool m_bEmailVerificationCompleted = false;
    std::string m_sLastEmailRequested; // Track last email to prevent duplicates
    
    // Encryption support
    std::unique_ptr<MatrixCrypto> m_pCrypto;
    bool m_bEncryptionEnabled = false;
    std::map<std::string, bool> m_RoomEncryptionState; // Track per-room encryption state
    mutable std::mutex m_EncryptionStateMutex;
    
    
    // Helper functions
    void ProcessIncomingMessage(const std::string& sender, const std::string& content);
    void HandleMatrixEvents();
    void DisplayInGameMessage(const std::string& sender, const std::string& content);
    void QueueMessage(const std::string& sender, const std::string& content);
    
    // Cryptographically secure encryption for password storage using Olm
    std::string SimpleEncrypt(const std::string& text);
    std::string SimpleDecrypt(const std::string& encrypted);
    
    // Helper functions for secure encryption
    std::string GetSystemEntropy();
    bool GenerateSecureRandom(uint8_t* buffer, size_t length);
    std::string Base64Encode(const std::string& input);
    std::string Base64Decode(const std::string& input);
    
    // Matrix HTTP API calls
    bool HttpLogin(const std::string& username, const std::string& password);
    bool HttpJoinRoom();
    bool HttpSendMessage(const std::string& message);
    void HttpSync();
    
    // Registration helper functions
    void CreateAccountInternal(const std::string& username, const std::string& password, const std::string& sessionId = "");
    void CompleteRegistration(const std::string& response_text);
    void HandleRegistrationError(int status_code, const std::string& response_text);
    void ResetRegistrationState();
    bool IsEmailVerificationPending() const;
    
    // Device and key management
    bool HttpUploadDeviceKeys();
    bool HttpDownloadDeviceKeys(const std::string& user_id = "");
    bool HttpSendToDevice(const std::string& event_type, const std::string& target_user, const std::string& target_device, const std::string& content);
    bool HttpUploadOneTimeKeys();
    bool HttpUploadKeys(); // Unified key upload method
    bool HttpUploadInitialKeys(); // Combined device + one-time keys upload (Element-web compatible)
    bool HttpClaimKeys(const std::vector<std::string>& user_ids);
    bool HttpGetRoomMembers();
    
    // Room key sharing
    void SendPendingRoomKeys();
    bool HttpSendRoomKeyRequest(const std::string& room_id, const std::string& session_id, const std::string& sender_key); // Element-web compatible
    
public:
    CChat();
    ~CChat();
    
    // Connection management
    void Connect();
    void Disconnect();
    void CreateAccount(const std::string& username, const std::string& password);
    void CreateAccountWithEmail(const std::string& username, const std::string& password, const std::string& email);
    void SubmitEmailVerification(const std::string& token);
    void ShowRecaptcha();
    void CompleteRecaptcha();
    void Login(const std::string& username, const std::string& password);
    
    // Chat settings persistence
    void SaveChatCredentials();
    void LoadChatCredentials();
    
    // Public encryption methods for config system
    std::string EncryptPassword(const std::string& password);
    std::string DecryptPassword(const std::string& encryptedPassword);
    
    // Status information
    std::string GetStatus() const;
    bool IsRecaptchaRequired() const { return m_bRecaptchaRequired; }
    bool IsRecaptchaShowing() const { return m_bRecaptchaShowing; }
    
    // Message handling
    void SendMessage(const std::string& message);
    
    // Encryption
    bool InitializeEncryption();
    void EnableEncryption(bool enable) { m_bEncryptionEnabled = enable; }
    bool IsEncryptionEnabled() const { return m_bEncryptionEnabled && m_pCrypto != nullptr; }
    
    // Room encryption state management
    void SetRoomEncryption(const std::string& room_id, bool encrypted);
    bool IsRoomEncrypted(const std::string& room_id) const;
    bool ShouldEncryptMessage(const std::string& room_id) const;
    
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