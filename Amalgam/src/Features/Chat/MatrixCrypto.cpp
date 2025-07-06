#include "MatrixCrypto.h"
#include <random>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <cstring>

// Random number generation for Olm
static void random_buffer(uint8_t* buffer, size_t length) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<unsigned int> dis(0, 255);
    
    for (size_t i = 0; i < length; ++i) {
        buffer[i] = static_cast<uint8_t>(dis(gen));
    }
}

MegolmSession::~MegolmSession()
{
    // Cleanup real Olm sessions
    if (outbound_session) {
        olm_clear_outbound_group_session(outbound_session);
        delete[] reinterpret_cast<uint8_t*>(outbound_session);
    }
    for (auto& [id, session] : inbound_sessions) {
        if (session) {
            olm_clear_inbound_group_session(session);
            delete[] reinterpret_cast<uint8_t*>(session);
        }
    }
}

MatrixCrypto::MatrixCrypto()
{
}

MatrixCrypto::~MatrixCrypto()
{
    Cleanup();
}

bool MatrixCrypto::Initialize(const std::string& user_id)
{
    m_sUserId = user_id;
    m_sDeviceId = GenerateDeviceId();
    
    // TODO: Initialize Olm account
    // For now, just return true as a placeholder
    return InitializeOlmAccount();
}

void MatrixCrypto::Cleanup()
{
    // TODO: Cleanup all Olm objects
    std::lock_guard<std::mutex> lock1(m_SessionsMutex);
    std::lock_guard<std::mutex> lock2(m_OlmSessionsMutex);
    
    m_RoomSessions.clear();
    // TODO: Clear Olm sessions
}

std::string MatrixCrypto::GenerateDeviceId()
{
    // Generate a random device ID
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    
    std::stringstream ss;
    ss << "AMALGAM";
    for (int i = 0; i < 10; ++i) {
        ss << std::hex << dis(gen);
    }
    
    return ss.str();
}

bool MatrixCrypto::InitializeOlmAccount()
{
    try {
        // Create Olm account
        size_t account_size = olm_account_size();
        m_pAccount = reinterpret_cast<OlmAccount*>(new uint8_t[account_size]);
        olm_account(m_pAccount);
        
        // Generate random bytes for account creation
        size_t random_length = olm_create_account_random_length(m_pAccount);
        std::vector<uint8_t> random_bytes(random_length);
        random_buffer(random_bytes.data(), random_length);
        
        // Create the account
        size_t result = olm_create_account(m_pAccount, random_bytes.data(), random_length);
        if (result == olm_error()) {
            delete[] reinterpret_cast<uint8_t*>(m_pAccount);
            m_pAccount = nullptr;
            return false;
        }
        
        return true;
    } catch (...) {
        if (m_pAccount) {
            delete[] reinterpret_cast<uint8_t*>(m_pAccount);
            m_pAccount = nullptr;
        }
        return false;
    }
}

std::string MatrixCrypto::GetDeviceKeys()
{
    if (!m_pAccount) {
        return "";
    }
    
    try {
        // Get identity keys from Olm account
        size_t id_keys_length = olm_account_identity_keys_length(m_pAccount);
        std::vector<uint8_t> id_keys_buffer(id_keys_length);
        
        size_t result = olm_account_identity_keys(m_pAccount, id_keys_buffer.data(), id_keys_length);
        if (result == olm_error()) {
            return "";
        }
        
        // Parse the identity keys JSON
        std::string id_keys_str(reinterpret_cast<char*>(id_keys_buffer.data()), id_keys_length);
        auto id_keys_json = nlohmann::json::parse(id_keys_str);
        
        // Create device keys JSON
        nlohmann::json keys = {
            {"user_id", m_sUserId},
            {"device_id", m_sDeviceId},
            {"algorithms", {"m.olm.v1.curve25519-aes-sha2", "m.megolm.v1.aes-sha2"}},
            {"keys", {
                {"curve25519:" + m_sDeviceId, id_keys_json["curve25519"]},
                {"ed25519:" + m_sDeviceId, id_keys_json["ed25519"]}
            }}
        };
        
        // Create canonical JSON for signing
        std::string canonical_json = keys.dump();
        
        // Sign the keys
        size_t signature_length = olm_account_signature_length(m_pAccount);
        std::vector<uint8_t> signature_buffer(signature_length);
        
        result = olm_account_sign(m_pAccount, 
                                 reinterpret_cast<const uint8_t*>(canonical_json.c_str()), 
                                 canonical_json.length(),
                                 signature_buffer.data(), 
                                 signature_length);
        
        if (result == olm_error()) {
            return keys.dump(); // Return unsigned keys if signing fails
        }
        
        // Add signature
        std::string signature_str(reinterpret_cast<char*>(signature_buffer.data()), signature_length);
        keys["signatures"] = {
            {m_sUserId, {
                {"ed25519:" + m_sDeviceId, signature_str}
            }}
        };
        
        return keys.dump();
    } catch (...) {
        return "";
    }
}

bool MatrixCrypto::UpdateUserDevices(const std::string& user_id, const std::map<std::string, MatrixDevice>& devices)
{
    std::lock_guard<std::mutex> lock(m_DevicesMutex);
    m_UserDevices[user_id] = devices;
    return true;
}

std::string MatrixCrypto::EncryptMessage(const std::string& room_id, const std::string& event_type, const std::string& content)
{
    // Ensure we have a session for this room
    if (!EnsureRoomSession(room_id)) {
        return ""; // Fallback to unencrypted
    }
    
    std::lock_guard<std::mutex> lock(m_SessionsMutex);
    auto it = m_RoomSessions.find(room_id);
    if (it == m_RoomSessions.end() || !it->second->outbound_session) {
        return "";
    }
    
    try {
        auto& session = it->second;
        
        // Get our curve25519 key for sender_key
        std::string sender_key;
        if (m_pAccount) {
            size_t id_keys_length = olm_account_identity_keys_length(m_pAccount);
            std::vector<uint8_t> id_keys_buffer(id_keys_length);
            
            if (olm_account_identity_keys(m_pAccount, id_keys_buffer.data(), id_keys_length) != olm_error()) {
                std::string id_keys_str(reinterpret_cast<char*>(id_keys_buffer.data()), id_keys_length);
                auto id_keys_json = nlohmann::json::parse(id_keys_str);
                sender_key = id_keys_json["curve25519"];
            }
        }
        
        // Create proper Matrix event structure for plaintext
        nlohmann::json plaintext_event = {
            {"type", event_type},
            {"content", nlohmann::json::parse(content)},
            {"room_id", room_id}
        };
        
        std::string plaintext = plaintext_event.dump();
        
        // Encrypt the message
        size_t ciphertext_length = olm_group_encrypt_message_length(session->outbound_session, plaintext.length());
        std::vector<uint8_t> ciphertext_buffer(ciphertext_length);
        
        size_t result = olm_group_encrypt(session->outbound_session,
                                         reinterpret_cast<const uint8_t*>(plaintext.c_str()),
                                         plaintext.length(),
                                         ciphertext_buffer.data(),
                                         ciphertext_length);
        
        if (result == olm_error()) {
            return "";
        }
        
        std::string ciphertext(reinterpret_cast<char*>(ciphertext_buffer.data()), result);
        
        // Create encrypted event
        nlohmann::json encrypted_content = {
            {"algorithm", "m.megolm.v1.aes-sha2"},
            {"ciphertext", ciphertext},
            {"sender_key", sender_key},
            {"session_id", session->session_id},
            {"device_id", m_sDeviceId}
        };
        
        // Increment message count
        session->message_count++;
        
        return encrypted_content.dump();
    } catch (...) {
        return "";
    }
}

std::string MatrixCrypto::DecryptMessage(const std::string& room_id, const std::string& encrypted_event)
{
    try {
        auto json = nlohmann::json::parse(encrypted_event);
        
        // Check for required fields per Matrix spec
        if (!json.contains("algorithm") || !json.contains("ciphertext") || 
            !json.contains("session_id") || !json.contains("sender_key")) {
            return "[Invalid encrypted event format - missing required fields]";
        }
        
        std::string algorithm = json["algorithm"];
        if (algorithm != "m.megolm.v1.aes-sha2") {
            return "[Unsupported encryption algorithm: " + algorithm + "]";
        }
        
        std::string ciphertext = json["ciphertext"];
        std::string session_id = json["session_id"];
        std::string sender_key = json["sender_key"];
        
        // Find the room session
        std::lock_guard<std::mutex> lock(m_SessionsMutex);
        auto room_it = m_RoomSessions.find(room_id);
        if (room_it == m_RoomSessions.end()) {
            return "[No session for room]";
        }
        
        auto& room_session = room_it->second;
        
        // Check if we have an inbound session for this session_id
        auto session_it = room_session->inbound_sessions.find(session_id);
        if (session_it == room_session->inbound_sessions.end()) {
            // Try to create inbound session from our outbound session if it's the same
            if (room_session->outbound_session && room_session->session_id == session_id) {
                // Create inbound session from our own outbound session
                size_t inbound_size = olm_inbound_group_session_size();
                OlmInboundGroupSession* inbound_session = reinterpret_cast<OlmInboundGroupSession*>(new uint8_t[inbound_size]);
                olm_inbound_group_session(inbound_session);
                
                // Export session key from outbound session
                size_t key_length = olm_outbound_group_session_key_length(room_session->outbound_session);
                std::vector<uint8_t> session_key(key_length);
                
                size_t key_result = olm_outbound_group_session_key(room_session->outbound_session,
                                                                 session_key.data(), key_length);
                if (key_result == olm_error()) {
                    delete[] reinterpret_cast<uint8_t*>(inbound_session);
                    return "[Failed to export session key]";
                }
                
                // Create inbound session from the key
                size_t result = olm_init_inbound_group_session(inbound_session,
                                                              session_key.data(), key_result);
                if (result == olm_error()) {
                    delete[] reinterpret_cast<uint8_t*>(inbound_session);
                    return "[Failed to create inbound session]";
                }
                
                // Store the inbound session
                room_session->inbound_sessions[session_id] = inbound_session;
                session_it = room_session->inbound_sessions.find(session_id);
            } else {
                return "[No inbound session for session_id: " + session_id + "]";
            }
        }
        
        // Decrypt the message
        OlmInboundGroupSession* inbound_session = session_it->second;
        
        // Create a mutable copy of ciphertext for olm_group_decrypt_max_plaintext_length
        std::vector<uint8_t> ciphertext_buffer_for_length(ciphertext.begin(), ciphertext.end());
        size_t max_plaintext_length = olm_group_decrypt_max_plaintext_length(inbound_session,
                                                                            ciphertext_buffer_for_length.data(),
                                                                            ciphertext_buffer_for_length.size());
        if (max_plaintext_length == olm_error()) {
            return "[Failed to get plaintext length]";
        }
        
        std::vector<uint8_t> plaintext_buffer(max_plaintext_length);
        std::vector<uint8_t> ciphertext_copy(ciphertext.begin(), ciphertext.end());
        uint32_t message_index;
        
        size_t plaintext_length = olm_group_decrypt(inbound_session,
                                                   ciphertext_copy.data(), ciphertext_copy.size(),
                                                   plaintext_buffer.data(), max_plaintext_length,
                                                   &message_index);
        
        if (plaintext_length == olm_error()) {
            const char* error = olm_inbound_group_session_last_error(inbound_session);
            return "[Decryption failed: " + std::string(error ? error : "unknown error") + "]";
        }
        
        std::string plaintext(reinterpret_cast<char*>(plaintext_buffer.data()), plaintext_length);
        
        // The plaintext should be a JSON event, extract the content
        try {
            auto plaintext_json = nlohmann::json::parse(plaintext);
            if (plaintext_json.contains("content") && plaintext_json["content"].contains("body")) {
                return plaintext_json["content"]["body"];
            } else {
                return plaintext; // Return raw plaintext if not standard message format
            }
        } catch (...) {
            return plaintext; // Return raw plaintext if JSON parsing fails
        }
        
    } catch (const std::exception& e) {
        return "[Decryption error: " + std::string(e.what()) + "]";
    } catch (...) {
        return "[Unknown decryption error]";
    }
}

bool MatrixCrypto::EnsureRoomSession(const std::string& room_id)
{
    std::lock_guard<std::mutex> lock(m_SessionsMutex);
    
    auto it = m_RoomSessions.find(room_id);
    if (it != m_RoomSessions.end()) {
        // Check if session needs rotation
        if (ShouldRotateSession(room_id)) {
            return RotateRoomSession(room_id);
        }
        return true;
    }
    
    return CreateMegolmSession(room_id);
}

bool MatrixCrypto::CreateMegolmSession(const std::string& room_id)
{
    try {
        auto session = std::make_unique<MegolmSession>();
        session->room_id = room_id;
        session->creation_time = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        
        // Create outbound group session
        size_t session_size = olm_outbound_group_session_size();
        session->outbound_session = reinterpret_cast<OlmOutboundGroupSession*>(new uint8_t[session_size]);
        olm_outbound_group_session(session->outbound_session);
        
        // Generate random bytes for session creation
        size_t random_length = olm_init_outbound_group_session_random_length(session->outbound_session);
        std::vector<uint8_t> random_bytes(random_length);
        random_buffer(random_bytes.data(), random_length);
        
        // Initialize the session
        size_t result = olm_init_outbound_group_session(session->outbound_session, 
                                                       random_bytes.data(), 
                                                       random_length);
        if (result == olm_error()) {
            return false;
        }
        
        // Get session ID
        size_t session_id_length = olm_outbound_group_session_id_length(session->outbound_session);
        std::vector<uint8_t> session_id_buffer(session_id_length);
        
        result = olm_outbound_group_session_id(session->outbound_session,
                                              session_id_buffer.data(),
                                              session_id_length);
        if (result == olm_error()) {
            return false;
        }
        
        session->session_id = std::string(reinterpret_cast<char*>(session_id_buffer.data()), session_id_length);
        
        m_RoomSessions[room_id] = std::move(session);
        return true;
    } catch (...) {
        return false;
    }
}

bool MatrixCrypto::RotateRoomSession(const std::string& room_id)
{
    // TODO: Rotate Megolm session
    return CreateMegolmSession(room_id);
}

bool MatrixCrypto::ShareSessionKey(const std::string& room_id, const std::vector<std::string>& user_ids)
{
    std::lock_guard<std::mutex> lock(m_SessionsMutex);
    auto it = m_RoomSessions.find(room_id);
    if (it == m_RoomSessions.end() || !it->second->outbound_session) {
        return false;
    }
    
    auto& session = it->second;
    
    // Only share the key if it hasn't been shared yet for this session
    // Or if the session is very new (within last 10 seconds) - re-share to ensure delivery
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    bool session_is_new = (now - session->creation_time) < 10000; // 10 seconds
    
    if (session->key_shared && !session_is_new) {
        return true; // Key already shared for this session and it's not brand new
    }
    
    try {
        // Export the session key
        size_t key_length = olm_outbound_group_session_key_length(session->outbound_session);
        std::vector<uint8_t> session_key_buffer(key_length);
        
        size_t key_result = olm_outbound_group_session_key(session->outbound_session,
                                                          session_key_buffer.data(), key_length);
        if (key_result == olm_error()) {
            return false;
        }
        
        std::string session_key(reinterpret_cast<char*>(session_key_buffer.data()), key_result);
        
        // Get our curve25519 key for sender_key
        std::string sender_key;
        if (m_pAccount) {
            size_t id_keys_length = olm_account_identity_keys_length(m_pAccount);
            std::vector<uint8_t> id_keys_buffer(id_keys_length);
            
            if (olm_account_identity_keys(m_pAccount, id_keys_buffer.data(), id_keys_length) != olm_error()) {
                std::string id_keys_str(reinterpret_cast<char*>(id_keys_buffer.data()), id_keys_length);
                auto id_keys_json = nlohmann::json::parse(id_keys_str);
                sender_key = id_keys_json["curve25519"];
            }
        }
        
        // Create room key event
        nlohmann::json room_key_event = {
            {"type", "m.room_key"},
            {"content", {
                {"algorithm", "m.megolm.v1.aes-sha2"},
                {"room_id", room_id},
                {"session_id", session->session_id},
                {"session_key", session_key},
                {"chain_index", 0}  // Always start from 0 so recipients can decrypt all messages
            }},
            {"sender", m_sUserId},
            {"sender_key", sender_key}
        };
        
        // Share room keys with all devices of specified users
        std::lock_guard<std::mutex> device_lock(m_DevicesMutex);
        int total_devices = 0;
        int successful_encryptions = 0;
        
        for (const auto& user_id : user_ids) {
            auto user_devices_it = m_UserDevices.find(user_id);
            if (user_devices_it != m_UserDevices.end()) {
                for (const auto& [device_id, device] : user_devices_it->second) {
                    total_devices++;
                    // Create Olm session if needed and encrypt the room key
                    if (CreateOlmSessionIfNeeded(device)) {
                        std::string encrypted_content = EncryptForDevice(device, room_key_event.dump());
                        if (!encrypted_content.empty()) {
                            // Store to be sent via to-device message
                            m_PendingRoomKeys.push_back({user_id, device_id, encrypted_content});
                            successful_encryptions++;
                        }
                    }
                }
            }
        }
        
        if (total_devices == 0) {
            // No devices found - this is likely the issue!
            return false;
        }
        
        // Mark the session key as shared
        session->key_shared = true;
        return true;
    } catch (...) {
        return false;
    }
}

bool MatrixCrypto::ProcessKeyShareEvent(const std::string& event)
{
    try {
        auto event_json = nlohmann::json::parse(event);
        
        if (!event_json.contains("type") || event_json["type"] != "m.room_key") {
            return false;
        }
        
        if (!event_json.contains("content")) {
            return false;
        }
        
        auto content = event_json["content"];
        if (!content.contains("algorithm") || content["algorithm"] != "m.megolm.v1.aes-sha2" ||
            !content.contains("room_id") || !content.contains("session_id") || !content.contains("session_key")) {
            return false;
        }
        
        std::string room_id = content["room_id"];
        std::string session_id = content["session_id"];
        std::string session_key = content["session_key"];
        
        // Create inbound group session from the received key
        size_t inbound_size = olm_inbound_group_session_size();
        OlmInboundGroupSession* inbound_session = reinterpret_cast<OlmInboundGroupSession*>(new uint8_t[inbound_size]);
        olm_inbound_group_session(inbound_session);
        
        size_t result = olm_init_inbound_group_session(inbound_session,
                                                      reinterpret_cast<const uint8_t*>(session_key.c_str()),
                                                      session_key.length());
        if (result == olm_error()) {
            delete[] reinterpret_cast<uint8_t*>(inbound_session);
            return false;
        }
        
        // Store the inbound session
        std::lock_guard<std::mutex> lock(m_SessionsMutex);
        
        // Find or create room session
        auto room_it = m_RoomSessions.find(room_id);
        if (room_it == m_RoomSessions.end()) {
            // Create new room session entry
            auto room_session = std::make_unique<MegolmSession>();
            room_session->room_id = room_id;
            room_session->session_id = session_id;
            room_session->creation_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            
            room_session->inbound_sessions[session_id] = inbound_session;
            m_RoomSessions[room_id] = std::move(room_session);
        } else {
            // Add to existing room session
            room_it->second->inbound_sessions[session_id] = inbound_session;
        }
        
        return true;
        
    } catch (...) {
        return false;
    }
}

bool MatrixCrypto::ShouldRotateSession(const std::string& room_id)
{
    auto it = m_RoomSessions.find(room_id);
    if (it == m_RoomSessions.end()) {
        return false;
    }
    
    auto& session = it->second;
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    // Rotate after 100 messages or 1 week
    const uint32_t MAX_MESSAGES = 100;
    const int64_t MAX_TIME_MS = 7 * 24 * 60 * 60 * 1000; // 1 week
    
    return (session->message_count >= MAX_MESSAGES) || 
           ((now - session->creation_time) >= MAX_TIME_MS);
}

void MatrixCrypto::CleanupOldSessions()
{
    // TODO: Cleanup old sessions
}

bool MatrixCrypto::CreateOlmSessionIfNeeded(const MatrixDevice& device)
{
    std::lock_guard<std::mutex> lock(m_OlmSessionsMutex);
    
    // Check if we already have a session for this device
    auto session_it = m_OlmSessions.find(device.curve25519_key);
    if (session_it != m_OlmSessions.end()) {
        return true; // Session already exists
    }
    
    try {
        // Create new Olm session
        size_t session_size = olm_session_size();
        OlmSession* session = reinterpret_cast<OlmSession*>(new uint8_t[session_size]);
        olm_session(session);
        
        // Get claimed one-time key for this device
        std::string one_time_key;
        {
            std::lock_guard<std::mutex> key_lock(m_ClaimedKeysMutex);
            
            // First try to find by curve25519 key (old method)
            auto key_it = m_ClaimedOneTimeKeys.find(device.curve25519_key);
            if (key_it != m_ClaimedOneTimeKeys.end()) {
                one_time_key = key_it->second;
            } else {
                // Try to find by user:device format (new method)
                std::string device_identifier = device.user_id + ":" + device.device_id;
                key_it = m_ClaimedOneTimeKeys.find(device_identifier);
                if (key_it != m_ClaimedOneTimeKeys.end()) {
                    one_time_key = key_it->second;
                } else {
                    // No claimed key available - use device key as fallback
                    one_time_key = device.curve25519_key;
                }
            }
        }
        
        std::string identity_key = device.curve25519_key;
        
        // Generate random bytes for session creation
        size_t random_length = olm_create_outbound_session_random_length(session);
        std::vector<uint8_t> random_buffer_data(random_length);
        random_buffer(random_buffer_data.data(), random_length);
        
        // Create outbound session with claimed one-time key
        size_t result = olm_create_outbound_session(
            session,
            m_pAccount,
            reinterpret_cast<const uint8_t*>(identity_key.c_str()), identity_key.length(),
            reinterpret_cast<const uint8_t*>(one_time_key.c_str()), one_time_key.length(),
            random_buffer_data.data(), random_length
        );
        
        if (result == olm_error()) {
            delete[] reinterpret_cast<uint8_t*>(session);
            return false;
        }
        
        // Store the session
        m_OlmSessions[device.curve25519_key] = session;
        return true;
        
    } catch (...) {
        return false;
    }
}

std::string MatrixCrypto::EncryptForDevice(const MatrixDevice& device, const std::string& content)
{
    std::lock_guard<std::mutex> lock(m_OlmSessionsMutex);
    
    auto session_it = m_OlmSessions.find(device.curve25519_key);
    if (session_it == m_OlmSessions.end()) {
        return "";
    }
    
    try {
        OlmSession* session = session_it->second;
        
        // Get the encrypt size
        size_t ciphertext_length = olm_encrypt_message_length(session, content.length());
        std::vector<uint8_t> ciphertext_buffer(ciphertext_length);
        
        // Generate random bytes for encryption
        size_t random_length = olm_encrypt_random_length(session);
        std::vector<uint8_t> random_buffer(random_length);
        
        // Fill with random data
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<unsigned int> dis(0, 255);
        for (auto& byte : random_buffer) {
            byte = static_cast<uint8_t>(dis(gen));
        }
        
        // Encrypt the message
        size_t message_type = olm_encrypt_message_type(session);
        size_t result = olm_encrypt(session,
                                   reinterpret_cast<const uint8_t*>(content.c_str()), content.length(),
                                   random_buffer.data(), random_length,
                                   ciphertext_buffer.data(), ciphertext_length);
        
        if (result == olm_error()) {
            return "";
        }
        
        std::string ciphertext(reinterpret_cast<char*>(ciphertext_buffer.data()), result);
        
        // Get our curve25519 key for sender_key
        std::string our_sender_key;
        if (m_pAccount) {
            size_t id_keys_length = olm_account_identity_keys_length(m_pAccount);
            std::vector<uint8_t> id_keys_buffer(id_keys_length);
            
            if (olm_account_identity_keys(m_pAccount, id_keys_buffer.data(), id_keys_length) != olm_error()) {
                std::string id_keys_str(reinterpret_cast<char*>(id_keys_buffer.data()), id_keys_length);
                auto id_keys_json = nlohmann::json::parse(id_keys_str);
                our_sender_key = id_keys_json["curve25519"];
            }
        }
        
        // Create encrypted event for to-device message
        nlohmann::json encrypted_event = {
            {"algorithm", "m.olm.v1.curve25519-aes-sha2"},
            {"ciphertext", {
                {device.curve25519_key, {
                    {"type", static_cast<int>(message_type)},
                    {"body", ciphertext}
                }}
            }},
            {"sender_key", our_sender_key}
        };
        
        return encrypted_event.dump();
        
    } catch (...) {
        return "";
    }
}

std::vector<PendingRoomKey> MatrixCrypto::GetPendingRoomKeys()
{
    std::lock_guard<std::mutex> lock(m_PendingKeysMutex);
    return m_PendingRoomKeys;
}

void MatrixCrypto::ClearPendingRoomKeys()
{
    std::lock_guard<std::mutex> lock(m_PendingKeysMutex);
    m_PendingRoomKeys.clear();
}

std::string MatrixCrypto::GetOneTimeKeys()
{
    if (!m_pAccount) {
        return "";
    }
    
    try {
        // Generate one-time keys if we don't have enough
        size_t max_keys = olm_account_max_number_of_one_time_keys(m_pAccount);
        size_t random_length = olm_account_generate_one_time_keys_random_length(m_pAccount, max_keys);
        std::vector<uint8_t> random_buffer_data(random_length);
        random_buffer(random_buffer_data.data(), random_length);
        
        size_t result = olm_account_generate_one_time_keys(m_pAccount, max_keys, random_buffer_data.data(), random_length);
        if (result == olm_error()) {
            return "";
        }
        
        // Get the one-time keys
        size_t keys_length = olm_account_one_time_keys_length(m_pAccount);
        std::vector<uint8_t> keys_buffer(keys_length);
        
        result = olm_account_one_time_keys(m_pAccount, keys_buffer.data(), keys_length);
        if (result == olm_error()) {
            return "";
        }
        
        std::string keys_str(reinterpret_cast<char*>(keys_buffer.data()), keys_length);
        auto keys_json = nlohmann::json::parse(keys_str);
        
        // Format for Matrix upload
        nlohmann::json one_time_keys;
        for (auto& [key_id, key_value] : keys_json["curve25519"].items()) {
            one_time_keys["curve25519:" + key_id] = key_value;
        }
        
        return one_time_keys.dump();
    } catch (...) {
        return "";
    }
}

void MatrixCrypto::MarkOneTimeKeysAsPublished()
{
    if (m_pAccount) {
        olm_account_mark_keys_as_published(m_pAccount);
    }
}

void MatrixCrypto::StoreClaimedKey(const std::string& device_key, const std::string& one_time_key)
{
    std::lock_guard<std::mutex> lock(m_ClaimedKeysMutex);
    m_ClaimedOneTimeKeys[device_key] = one_time_key;
}

std::string MatrixCrypto::DecryptToDeviceMessage(const std::string& encrypted_event)
{
    try {
        auto json = nlohmann::json::parse(encrypted_event);
        
        // Check for required fields per Matrix spec
        if (!json.contains("algorithm") || !json.contains("ciphertext") || !json.contains("sender_key")) {
            return "[Invalid encrypted to-device event format]";
        }
        
        std::string algorithm = json["algorithm"];
        if (algorithm != "m.olm.v1.curve25519-aes-sha2") {
            return "[Unsupported to-device encryption algorithm: " + algorithm + "]";
        }
        
        std::string sender_key = json["sender_key"];
        
        // Get our curve25519 key to find the correct ciphertext
        std::string our_key;
        if (m_pAccount) {
            size_t id_keys_length = olm_account_identity_keys_length(m_pAccount);
            std::vector<uint8_t> id_keys_buffer(id_keys_length);
            
            if (olm_account_identity_keys(m_pAccount, id_keys_buffer.data(), id_keys_length) != olm_error()) {
                std::string id_keys_str(reinterpret_cast<char*>(id_keys_buffer.data()), id_keys_length);
                auto id_keys_json = nlohmann::json::parse(id_keys_str);
                our_key = id_keys_json["curve25519"];
            }
        }
        
        if (our_key.empty()) {
            return "[Could not get our device key]";
        }
        
        // Find the ciphertext intended for us
        if (!json["ciphertext"].contains(our_key)) {
            return "[Message not intended for this device]";
        }
        
        auto ciphertext_obj = json["ciphertext"][our_key];
        if (!ciphertext_obj.contains("type") || !ciphertext_obj.contains("body")) {
            return "[Invalid ciphertext format]";
        }
        
        int message_type = ciphertext_obj["type"];
        std::string ciphertext_body = ciphertext_obj["body"];
        
        // Find or create Olm session for this sender
        std::lock_guard<std::mutex> lock(m_OlmSessionsMutex);
        auto session_it = m_OlmSessions.find(sender_key);
        
        OlmSession* session = nullptr;
        if (session_it == m_OlmSessions.end()) {
            // If it's a pre-key message (type 0), create inbound session
            if (message_type == 0) {
                size_t session_size = olm_session_size();
                session = reinterpret_cast<OlmSession*>(new uint8_t[session_size]);
                olm_session(session);
                
                // Create inbound session from pre-key message
                std::vector<uint8_t> ciphertext_buffer(ciphertext_body.begin(), ciphertext_body.end());
                size_t result = olm_create_inbound_session(session, m_pAccount, 
                                                          ciphertext_buffer.data(), ciphertext_buffer.size());
                
                if (result == olm_error()) {
                    delete[] reinterpret_cast<uint8_t*>(session);
                    return "[Failed to create inbound session]";
                }
                
                // Remove the one-time key that was used
                olm_remove_one_time_keys(m_pAccount, session);
                
                // Store the session
                m_OlmSessions[sender_key] = session;
            } else {
                return "[No session available for sender and message is not pre-key]";
            }
        } else {
            session = session_it->second;
        }
        
        // Decrypt the message
        std::vector<uint8_t> ciphertext_buffer(ciphertext_body.begin(), ciphertext_body.end());
        size_t max_plaintext_length = olm_decrypt_max_plaintext_length(session, message_type, 
                                                                      ciphertext_buffer.data(), ciphertext_buffer.size());
        
        if (max_plaintext_length == olm_error()) {
            return "[Failed to get max plaintext length]";
        }
        
        std::vector<uint8_t> plaintext_buffer(max_plaintext_length);
        std::vector<uint8_t> ciphertext_copy(ciphertext_body.begin(), ciphertext_body.end());
        
        size_t plaintext_length = olm_decrypt(session, message_type,
                                             ciphertext_copy.data(), ciphertext_copy.size(),
                                             plaintext_buffer.data(), max_plaintext_length);
        
        if (plaintext_length == olm_error()) {
            const char* error = olm_session_last_error(session);
            return "[To-device decryption failed: " + std::string(error ? error : "unknown error") + "]";
        }
        
        std::string plaintext(reinterpret_cast<char*>(plaintext_buffer.data()), plaintext_length);
        return plaintext;
        
    } catch (const std::exception& e) {
        return "[To-device decryption error: " + std::string(e.what()) + "]";
    } catch (...) {
        return "[Unknown to-device decryption error]";
    }
}