#pragma once

#include <string>
#include <map>
#include <memory>
#include <mutex>
#include <vector>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <queue>
#include <atomic>
#include <functional>
#include <thread>
#include <nlohmann/json.hpp>
#include <condition_variable>

// For C++14 compatibility - fallback for older compilers
#if __cplusplus >= 201402L || _MSC_VER >= 1900
    #include <shared_mutex>
    #define HAVE_SHARED_MUTEX 1
    namespace crypto_mutex {
        using shared_mutex = std::shared_mutex;
        using shared_lock = std::shared_lock<std::shared_mutex>;
        using unique_lock = std::unique_lock<std::shared_mutex>;
    }
#else
    #define HAVE_SHARED_MUTEX 0
    namespace crypto_mutex {
        using shared_mutex = std::mutex;
        using shared_lock = std::lock_guard<std::mutex>;
        using unique_lock = std::lock_guard<std::mutex>;
    }
#endif

// JSON library - check if available
#include <nlohmann/json.hpp>

// Include Olm headers from External directory
extern "C" {
#include "../../External/libolm/libolm/include/olm/olm.h"
#include "../../External/libolm/libolm/include/olm/outbound_group_session.h"
#include "../../External/libolm/libolm/include/olm/inbound_group_session.h"
#include "../../External/libolm/libolm/include/olm/pk.h"
#include "../../External/libolm/libolm/include/olm/sas.h"
#include "../../External/libolm/libolm/include/olm/base64.h"
#include "../../External/libolm/libolm/include/olm/error.h"
#include "../../External/libolm/libolm/include/olm/crypto.h"
}

// Error handling system matching matrix-sdk-crypto-wasm
enum class DecryptionErrorCode {
    MissingRoomKey,
    UnknownMessageIndex,
    MismatchedIdentityKeys,
    UnknownSenderDevice,
    UnsignedSenderDevice,
    SenderIdentityVerificationViolation,
    UnableToDecrypt,
    MismatchedSender,
    SessionCorrupted,
    InvalidCiphertext,
    RatchetError,
    UnknownError
};

// Device verification status
enum class DeviceVerificationStatus {
    Unverified,
    Verified,
    BlackListed,
    CrossSigningTrusted,
    LocallyTrusted
};

// Matrix device information with enhanced verification
struct MatrixDevice
{
    std::string device_id;
    std::string user_id;
    std::string curve25519_key;
    std::string ed25519_key;
    DeviceVerificationStatus verification_status = DeviceVerificationStatus::Unverified;
    std::string display_name;
    std::map<std::string, std::map<std::string, std::string>> signatures;
    int64_t first_time_seen = 0;
    int64_t last_seen = 0;
    bool deleted = false;
    
    // Cross-signing information
    bool cross_signing_trusted = false;
    std::string cross_signing_key;
};

// Message ordering and duplicate detection
struct MessageMetadata {
    std::string event_id;
    std::string sender_key;
    std::string session_id;
    uint32_t message_index;
    int64_t timestamp;
    std::string room_id;
    bool processed = false;
};

// Inbound session with metadata
struct InboundGroupSession {
    OlmInboundGroupSession* session = nullptr;
    std::string session_id;
    std::string sender_key;
    std::string room_id;
    uint32_t first_known_index = 0;
    bool forwarded = false;
    std::string export_format_key;
    int64_t creation_time = 0;
    
    // Message tracking for duplicate detection
    std::set<uint32_t> processed_indices;
    std::map<uint32_t, MessageMetadata> message_metadata;
    
    ~InboundGroupSession() {
        if (session) {
            olm_clear_inbound_group_session(session);
            delete[] reinterpret_cast<uint8_t*>(session);
        }
    }
};

// Enhanced Megolm session for room encryption
struct MegolmSession
{
    std::string session_id;
    std::string room_id;
    OlmOutboundGroupSession* outbound_session = nullptr;
    std::map<std::string, std::unique_ptr<InboundGroupSession>> inbound_sessions; // key = session_id
    
    // Session state tracking
    uint32_t message_count = 0;
    int64_t creation_time = 0;
    int64_t last_use_time = 0;
    bool key_shared = false;
    bool needs_rotation = false;
    
    // Key sharing state
    std::string session_key_export_format;
    std::set<std::string> devices_with_key; // device_identifier set
    
    // Session rotation parameters
    static const uint32_t MAX_MESSAGES_PER_SESSION = 100;
    static const int64_t MAX_SESSION_AGE_MS = 7 * 24 * 60 * 60 * 1000; // 7 days
    
    ~MegolmSession();
};

// Enhanced room key sharing with retry logic
struct PendingRoomKey
{
    std::string user_id;
    std::string device_id;
    std::string encrypted_content;
    uint64_t sequence_number = 0;
    int64_t timestamp = 0;
    int retry_count = 0;
    std::string session_id;
    std::string room_id;
    
    // Retry logic parameters
    static const int MAX_RETRY_COUNT = 3;
    static const int64_t RETRY_DELAY_MS = 5000; // 5 seconds
};

// Enhanced Olm session tracking
struct OlmSessionInfo {
    OlmSession* session = nullptr;
    std::string session_id;
    std::string sender_key;
    bool has_received_message = false;
    uint32_t message_count = 0;
    int64_t creation_time = 0;
    int64_t last_use_time = 0;
    
    // Session health tracking
    bool corrupted = false;
    std::string last_error;
    
    ~OlmSessionInfo() {
        if (session) {
            olm_clear_session(session);
            delete[] reinterpret_cast<uint8_t*>(session);
        }
    }
};

// Enhanced device failure tracking
struct FailedDevice
{
    std::string user_id;
    std::string device_id;
    std::string curve25519_key;
    int64_t last_attempt_time = 0;
    int retry_count = 0;
    std::string failure_reason;
    DecryptionErrorCode error_code = DecryptionErrorCode::UnknownError;
    
    // Exponential backoff parameters
    static const int64_t BASE_RETRY_DELAY_MS = 1000; // 1 second
    static const int MAX_RETRY_COUNT = 5;
    static const int64_t MAX_RETRY_DELAY_MS = 30000; // 30 seconds
    
    int64_t GetNextRetryTime() const {
        int64_t delay = BASE_RETRY_DELAY_MS * (1LL << std::min(retry_count, 5));
        return last_attempt_time + std::min(delay, MAX_RETRY_DELAY_MS);
    }
};

// Cross-signing key information
struct CrossSigningKey {
    std::string key_id;
    std::string public_key;
    std::vector<std::string> usage; // ["master", "self_signing", "user_signing"]
    std::map<std::string, std::map<std::string, std::string>> signatures;
    bool verified = false;
};

// One-time key management
struct OneTimeKeyInfo {
    std::string key_id;
    std::string key;
    bool published = false;
    int64_t creation_time = 0;
};

class MatrixCrypto
{
private:
    // Core crypto state
    OlmAccount* m_pAccount = nullptr;
    std::string m_sDeviceId;
    std::string m_sUserId;
    std::atomic<bool> m_bInitialized{false};
    std::atomic<bool> m_bShuttingDown{false};
    
    // Device tracking with enhanced verification
    std::unordered_map<std::string, std::unordered_map<std::string, MatrixDevice>> m_UserDevices;
    mutable crypto_mutex::shared_mutex m_DevicesMutex;
    
    // Megolm sessions per room with enhanced state tracking
    std::unordered_map<std::string, std::unique_ptr<MegolmSession>> m_RoomSessions;
    mutable crypto_mutex::shared_mutex m_SessionsMutex;
    
    // Olm sessions for device-to-device communication with health tracking
    std::unordered_map<std::string, std::unique_ptr<OlmSessionInfo>> m_OlmSessions;
    mutable crypto_mutex::shared_mutex m_OlmSessionsMutex;
    
    // Message processing and ordering
    std::unordered_map<std::string, MessageMetadata> m_ProcessedMessages; // event_id -> metadata
    std::mutex m_MessageTrackingMutex;
    
    // Pending operations with retry logic
    std::queue<PendingRoomKey> m_PendingRoomKeys;
    std::mutex m_PendingKeysMutex;
    
    // One-time key management
    std::unordered_map<std::string, OneTimeKeyInfo> m_OneTimeKeys;
    std::mutex m_OneTimeKeysMutex;
    
    // Device failure tracking with exponential backoff
    std::unordered_map<std::string, FailedDevice> m_FailedDevices;
    mutable crypto_mutex::shared_mutex m_FailedDevicesMutex;
    
    // Cross-signing support
    std::unordered_map<std::string, CrossSigningKey> m_CrossSigningKeys; // key_id -> key
    std::mutex m_CrossSigningMutex;
    
    // Session rotation and cleanup
    std::atomic<uint64_t> m_MessageSequence{0};
    std::thread m_MaintenanceThread;
    std::condition_variable m_MaintenanceCV;
    std::mutex m_MaintenanceMutex;
    
    // Concurrent operation management
    std::unordered_set<std::string> m_ActiveOperations;
    std::mutex m_ActiveOperationsMutex;
    
    // Enhanced timing control for Element compatibility
    std::atomic<uint32_t> m_SessionKeyExportDelay{100}; // ms
    std::atomic<uint32_t> m_DeviceVerificationTimeout{30000}; // ms
    
    // Encryption settings per room
    std::unordered_map<std::string, nlohmann::json> m_RoomEncryptionSettings;
    std::mutex m_EncryptionSettingsMutex;
    
    // Key backup configuration
    std::string m_BackupKey;
    std::string m_BackupVersion;
    std::mutex m_BackupMutex;
    
    // Core initialization and cleanup
    bool InitializeOlmAccount();
    void StartMaintenanceThread();
    void StopMaintenanceThread();
    void MaintenanceLoop();
    
    // Device management
    std::string GenerateDeviceId();
    bool ValidateDeviceKeys(const MatrixDevice& device);
    bool VerifyDeviceSignature(const MatrixDevice& device);
    
    // Session management
    bool CreateMegolmSession(const std::string& room_id);
    bool RotateMegolmSession(const std::string& room_id);
    bool ShouldRotateSession(const std::string& room_id, const MegolmSession& session);
    
    // Olm session management
    bool CreateOlmSessionIfNeeded(const MatrixDevice& device);
    bool ValidateOlmSession(const std::string& sender_key, OlmSessionInfo* session_info);
    std::string EncryptForDevice(const MatrixDevice& device, const std::string& content);
    
    // Key sharing with enhanced error handling
    bool ShareRoomKey(const std::string& room_id, const std::vector<MatrixDevice>& devices);
    bool RetryFailedKeySharing(const std::string& room_id);
    
    // Message processing
    bool IsMessageProcessed(const std::string& event_id);
    void MarkMessageProcessed(const std::string& event_id, const MessageMetadata& metadata);
    
    // Error handling and recovery
    DecryptionErrorCode ClassifyDecryptionError(const std::string& error_msg);
    bool RecoverFromSessionError(const std::string& room_id, const std::string& session_id);
    
    // Cross-signing support
    bool ValidateCrossSigningKey(const CrossSigningKey& key);
    bool IsCrossSigningTrusted(const std::string& user_id, const std::string& device_id);
    
    // Utility functions
    std::string GetCanonicalJson(const nlohmann::json& json);
    bool VerifySignature(const std::string& message, const std::string& signature, const std::string& public_key);
    std::string SignMessage(const std::string& message);
    std::string GenerateSecureRandom(size_t length);
    
    // Session cleanup
    void CleanupExpiredSessions();
    void CleanupOldMessages();
    void CleanupFailedDevices();
    
public:
    MatrixCrypto();
    ~MatrixCrypto();
    
    // Initialization
    bool Initialize(const std::string& user_id);
    void Cleanup();
    
    // Device management
    std::string GetDeviceId() const { return m_sDeviceId; }
    OlmAccount* GetOlmAccount() const { return m_pAccount; }
    std::string GetDeviceKeys(); // Returns JSON with our device keys
    std::string GetOneTimeKeys(); // Returns JSON with one-time keys
    bool UpdateUserDevices(const std::string& user_id, const std::unordered_map<std::string, MatrixDevice>& devices);
    bool UpdateUserDevices(const std::string& user_id, const std::map<std::string, MatrixDevice>& devices);
    std::unordered_map<std::string, MatrixDevice> GetUserDevices(const std::string& user_id) const;
    void MarkOneTimeKeysAsPublished();
    
    // Encryption/Decryption
    std::string EncryptMessage(const std::string& room_id, const std::string& event_type, const std::string& content);
    std::string DecryptMessage(const std::string& room_id, const std::string& encrypted_event);
    std::string DecryptToDeviceMessage(const std::string& encrypted_event);
    
    // Session management
    bool EnsureRoomSession(const std::string& room_id);
    bool RotateRoomSession(const std::string& room_id);
    
    // Key sharing
    bool ShareSessionKey(const std::string& room_id, const std::vector<std::string>& user_ids);
    bool ProcessKeyShareEvent(const std::string& event);
    
    // Utilities
    bool ShouldRotateSession(const std::string& room_id);
    void CleanupOldSessions();
    
    // Enhanced debugging and diagnostics
    void LogOlmLibraryVersion();
    void LogCryptoState();
    bool ValidateSessionIntegrity(const std::string& room_id);
    
    // Advanced session management
    bool ImportSessionKey(const std::string& room_id, const std::string& session_key, uint32_t message_index);
    bool ExportSessionKey(const std::string& room_id, uint32_t message_index, std::string& exported_key);
    
    // Device key lifecycle
    bool GenerateOneTimeKeys(size_t count);
    bool UploadOneTimeKeys();
    bool ClaimOneTimeKey(const std::string& user_id, const std::string& device_id);
    void StoreClaimedKey(const std::string& device_key, const std::string& one_time_key);
    
    // Cross-signing operations
    bool SetupCrossSigning(const std::string& master_key, const std::string& self_signing_key, const std::string& user_signing_key);
    bool SignDevice(const std::string& user_id, const std::string& device_id);
    bool VerifyDeviceWithCrossSigning(const std::string& user_id, const std::string& device_id);
    
    // Enhanced failure management
    void AddFailedDevice(const std::string& user_id, const std::string& device_id, 
                        const std::string& curve25519_key, const std::string& reason, 
                        DecryptionErrorCode error_code = DecryptionErrorCode::UnknownError);
    bool ShouldRetryFailedDevice(const std::string& device_identifier);
    void ClearFailedDevices();
    
    // Session recovery and health
    bool RecoverRoomSession(const std::string& room_id);
    bool ForceSessionRotation(const std::string& room_id);
    bool ReshareSessionKeys(const std::string& room_id, const std::vector<std::string>& user_ids);
    bool TestSessionHealth(const std::string& room_id);
    
    // Pending operations management
    std::vector<PendingRoomKey> GetPendingRoomKeys();
    std::vector<PendingRoomKey> GetAndClearPendingRoomKeys();
    void ProcessPendingRoomKeys();
    void ClearPendingRoomKeys();
    
    // Message verification
    bool VerifyMessageAuthenticity(const std::string& event_id, const std::string& sender_key, 
                                  const std::string& content, const std::string& signature);
    
    // Backup and recovery
    bool BackupSessionKeys(const std::string& backup_key);
    bool RestoreSessionKeys(const std::string& backup_data, const std::string& backup_key);
    
    // Enhanced compatibility methods to match matrix-sdk-crypto-wasm
    bool ExportRoomKeys(std::string& exported_keys);
    bool ImportRoomKeys(const std::string& exported_keys, bool overwrite_existing = false);
    std::string GetEncryptionSettings(const std::string& room_id);
    bool SetEncryptionSettings(const std::string& room_id, const std::string& settings);
    
    // Device trust management matching Element's behavior
    bool TrustDevice(const std::string& user_id, const std::string& device_id);
    bool UntrustDevice(const std::string& user_id, const std::string& device_id);
    bool IsDeviceTrusted(const std::string& user_id, const std::string& device_id) const;
    
    // Session health and diagnostics
    bool ValidateAllSessions();
    std::string GetSessionInfo(const std::string& room_id);
    std::string GetDeviceInfo(const std::string& user_id, const std::string& device_id);
    
    // Key backup and recovery (Element compatibility)
    bool SetupKeyBackup(const std::string& backup_key, const std::string& backup_version);
    bool RestoreFromKeyBackup(const std::string& backup_key, const std::string& backup_data);
    
    // Advanced room key sharing with Element-like behavior
    bool ShareHistoricalKeys(const std::string& room_id, const std::vector<std::string>& user_ids);
    bool RequestRoomKey(const std::string& room_id, const std::string& session_id, const std::string& sender_key);
    
    // New operation tracking for better Element compatibility
    bool HasPendingOperations() const;
    size_t GetPendingOperationCount() const;
    void ProcessAllPendingOperations();
    
    // Timing and synchronization fixes for Element compatibility
    void FlushPendingOperations();
    bool WaitForPendingOperations(uint32_t timeout_ms = 5000);
    void SetSessionKeyExportTiming(uint32_t delay_ms = 100);
    void SetDeviceVerificationTimeout(uint32_t timeout_ms = 30000);
    
    // Utility methods for Element compatibility
    std::string GetOwnCurve25519Key() const;
    std::string GetOwnEd25519Key() const;
    
    // Session corruption recovery
    void ClearCorruptedSessions();
    void ClearAllOlmSessions();
    bool ForceRecreateOlmSession(const MatrixDevice& device);
    bool HandleKeyExhaustion();
    
    // Missing method declarations
    size_t GetFailedDeviceCount() const;
    bool RetryFailedDevices(const std::string& room_id);
    std::string DumpCryptoState() const;
    bool SelfTest();
    std::string GetLibraryVersions() const;
    
private:
    // Room key encryption for to-device messages
    std::string GenerateRoomKeyMessage(const MatrixDevice& device, const std::string& room_id);
};