#pragma once

#include <string>
#include <map>
#include <memory>
#include <mutex>
#include <vector>
#include <set>
#include <chrono>
#include <queue>
#include <atomic>
#include <thread>
#include <condition_variable>

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

// Error handling system
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

// Matrix device information
struct MatrixDevice
{
    std::string device_id;
    std::string user_id;
    std::string curve25519_key;
    std::string ed25519_key;
    DeviceVerificationStatus verification_status = DeviceVerificationStatus::Unverified;
    std::string display_name;
    std::map<std::string, std::string> signatures;
    int64_t first_time_seen = 0;
    int64_t last_seen = 0;
    bool deleted = false;
    bool cross_signing_trusted = false;
    std::string cross_signing_key;
};

// Simplified Megolm session
struct MegolmSession
{
    std::string session_id;
    std::string room_id;
    OlmOutboundGroupSession* outbound_session = nullptr;
    std::map<std::string, OlmInboundGroupSession*> inbound_sessions;
    uint32_t message_count = 0;
    int64_t creation_time = 0;
    int64_t last_use_time = 0;
    bool key_shared = false;
    bool needs_rotation = false;
    std::set<std::string> devices_with_key;
    
    // Session rotation parameters
    static const uint32_t MAX_MESSAGES_PER_SESSION = 100;
    static const int64_t MAX_SESSION_AGE_MS = 7 * 24 * 60 * 60 * 1000; // 7 days
    
    ~MegolmSession();
};

// Failed device tracking
struct FailedDevice
{
    std::string user_id;
    std::string device_id;
    std::string curve25519_key;
    int64_t last_attempt_time = 0;
    int retry_count = 0;
    std::string failure_reason;
    DecryptionErrorCode error_code = DecryptionErrorCode::UnknownError;
    
    static const int64_t BASE_RETRY_DELAY_MS = 1000;
    static const int MAX_RETRY_COUNT = 5;
    static const int64_t MAX_RETRY_DELAY_MS = 30000;
    
    int64_t GetNextRetryTime() const {
        int64_t delay = BASE_RETRY_DELAY_MS * (1 << (retry_count < 5 ? retry_count : 5));
        return last_attempt_time + (delay < MAX_RETRY_DELAY_MS ? delay : MAX_RETRY_DELAY_MS);
    }
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
    
    // Device tracking (simplified with regular mutex)
    std::map<std::string, std::map<std::string, MatrixDevice>> m_UserDevices;
    mutable std::mutex m_DevicesMutex;
    
    // Megolm sessions per room
    std::map<std::string, std::unique_ptr<MegolmSession>> m_RoomSessions;
    mutable std::mutex m_SessionsMutex;
    
    // Failed devices tracking
    std::map<std::string, FailedDevice> m_FailedDevices;
    mutable std::mutex m_FailedDevicesMutex;
    
    // Session rotation and cleanup
    std::atomic<uint64_t> m_MessageSequence{0};
    std::thread m_MaintenanceThread;
    std::condition_variable m_MaintenanceCV;
    std::mutex m_MaintenanceMutex;
    
    // Helper functions
    bool InitializeOlmAccount();
    void StartMaintenanceThread();
    void StopMaintenanceThread();
    void MaintenanceLoop();
    
    // Device management
    std::string GenerateDeviceId();
    bool ValidateDeviceKeys(const MatrixDevice& device);
    
    // Session management
    bool CreateMegolmSession(const std::string& room_id);
    bool RotateMegolmSession(const std::string& room_id);
    bool ShouldRotateSession(const std::string& room_id, const MegolmSession& session);
    
    // Error handling
    DecryptionErrorCode ClassifyDecryptionError(const std::string& error_msg);
    
    // Utility functions
    std::string GetOwnCurve25519Key();
    
    // Session cleanup
    void CleanupExpiredSessions();
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
    std::string GetDeviceKeys();
    std::string GetOneTimeKeys();
    bool UpdateUserDevices(const std::string& user_id, const std::map<std::string, MatrixDevice>& devices);
    std::map<std::string, MatrixDevice> GetUserDevices(const std::string& user_id) const;
    void MarkOneTimeKeysAsPublished();
    bool GenerateOneTimeKeys(size_t count);
    void StoreClaimedKey(const std::string& device_key, const std::string& one_time_key);
    
    // Encryption/Decryption
    std::string EncryptMessage(const std::string& room_id, const std::string& event_type, const std::string& content);
    std::string DecryptMessage(const std::string& room_id, const std::string& encrypted_event);
    std::string DecryptToDeviceMessage(const std::string& encrypted_event);
    
    // Session management
    bool EnsureRoomSession(const std::string& room_id);
    bool RotateRoomSession(const std::string& room_id);
    bool ShouldRotateSession(const std::string& room_id);
    
    // Key sharing
    bool ShareSessionKey(const std::string& room_id, const std::vector<std::string>& user_ids);
    bool ProcessKeyShareEvent(const std::string& event);
    
    // Session recovery
    bool RecoverRoomSession(const std::string& room_id);
    bool ForceSessionRotation(const std::string& room_id);
    bool ValidateSessionIntegrity(const std::string& room_id);
    
    // Failed device management
    void AddFailedDevice(const std::string& user_id, const std::string& device_id, 
                        const std::string& curve25519_key, const std::string& reason, 
                        DecryptionErrorCode error_code = DecryptionErrorCode::UnknownError);
    bool RetryFailedDevices(const std::string& room_id);
    void ClearFailedDevices();
    size_t GetFailedDeviceCount() const;
    
    // Utilities
    void CleanupOldSessions();
    void LogOlmLibraryVersion();
    void LogCryptoState();
};

ADD_FEATURE(MatrixCrypto, MatrixCrypto)