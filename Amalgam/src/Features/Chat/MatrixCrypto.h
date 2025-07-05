#pragma once

#include <string>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

// Include Olm headers
extern "C" {
#include <olm/olm.h>
#include <olm/outbound_group_session.h>
#include <olm/inbound_group_session.h>
#include <olm/pk.h>
#include <olm/sas.h>
}

// Matrix device information
struct MatrixDevice
{
    std::string device_id;
    std::string user_id;
    std::string curve25519_key;
    std::string ed25519_key;
    bool verified = false;
};

// Megolm session for room encryption
struct MegolmSession
{
    std::string session_id;
    std::string room_id;
    OlmOutboundGroupSession* outbound_session = nullptr;
    std::map<std::string, OlmInboundGroupSession*> inbound_sessions; // key = session_id
    uint32_t message_count = 0;
    int64_t creation_time = 0;
    
    ~MegolmSession();
};

// Pending room key for to-device sending
struct PendingRoomKey
{
    std::string user_id;
    std::string device_id;
    std::string encrypted_content;
};

class MatrixCrypto
{
private:
    // Olm account for this device
    OlmAccount* m_pAccount = nullptr;
    std::string m_sDeviceId;
    std::string m_sUserId;
    
    // Device tracking
    std::map<std::string, std::map<std::string, MatrixDevice>> m_UserDevices; // user_id -> device_id -> device
    std::mutex m_DevicesMutex;
    
    // Megolm sessions per room
    std::map<std::string, std::unique_ptr<MegolmSession>> m_RoomSessions; // room_id -> session
    std::mutex m_SessionsMutex;
    
    // Olm sessions for device-to-device communication
    std::map<std::string, OlmSession*> m_OlmSessions; // device_key -> session
    std::mutex m_OlmSessionsMutex;
    
    // Pending room keys to be sent via to-device messages
    std::vector<PendingRoomKey> m_PendingRoomKeys;
    std::mutex m_PendingKeysMutex;
    
    // Claimed one-time keys for session creation
    std::map<std::string, std::string> m_ClaimedOneTimeKeys; // device_key -> one_time_key
    std::mutex m_ClaimedKeysMutex;
    
    // Helper functions
    bool InitializeOlmAccount();
    std::string GenerateDeviceId();
    bool CreateMegolmSession(const std::string& room_id);
    bool ShareRoomKey(const std::string& room_id, const std::vector<MatrixDevice>& devices);
    bool CreateOlmSessionIfNeeded(const MatrixDevice& device);
    std::string EncryptForDevice(const MatrixDevice& device, const std::string& content);
    
public:
    MatrixCrypto();
    ~MatrixCrypto();
    
    // Initialization
    bool Initialize(const std::string& user_id);
    void Cleanup();
    
    // Device management
    std::string GetDeviceId() const { return m_sDeviceId; }
    std::string GetDeviceKeys(); // Returns JSON with our device keys
    std::string GetOneTimeKeys(); // Returns JSON with one-time keys
    bool UpdateUserDevices(const std::string& user_id, const std::map<std::string, MatrixDevice>& devices);
    void MarkOneTimeKeysAsPublished();
    
    // Encryption/Decryption
    std::string EncryptMessage(const std::string& room_id, const std::string& event_type, const std::string& content);
    std::string DecryptMessage(const std::string& room_id, const std::string& encrypted_event);
    
    // Session management
    bool EnsureRoomSession(const std::string& room_id);
    bool RotateRoomSession(const std::string& room_id);
    
    // Key sharing
    bool ShareSessionKey(const std::string& room_id, const std::vector<std::string>& user_ids);
    bool ProcessKeyShareEvent(const std::string& event);
    
    // Utilities
    bool ShouldRotateSession(const std::string& room_id);
    void CleanupOldSessions();
    
    // Pending room key management
    std::vector<PendingRoomKey> GetPendingRoomKeys();
    void ClearPendingRoomKeys();
    
    // One-time key management
    void StoreClaimedKey(const std::string& device_key, const std::string& one_time_key);
};