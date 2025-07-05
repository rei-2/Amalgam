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
    std::uniform_int_distribution<> dis(0, 255);
    
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
        
        // Encrypt the message
        size_t ciphertext_length = olm_group_encrypt_message_length(session->outbound_session, content.length());
        std::vector<uint8_t> ciphertext_buffer(ciphertext_length);
        
        size_t result = olm_group_encrypt(session->outbound_session,
                                         reinterpret_cast<const uint8_t*>(content.c_str()),
                                         content.length(),
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
    // TODO: Actual decryption using Megolm
    // For now, return placeholder
    try {
        auto json = nlohmann::json::parse(encrypted_event);
        if (json.contains("ciphertext")) {
            std::string ciphertext = json["ciphertext"];
            if (ciphertext.find("placeholder_encrypted_") == 0) {
                // Remove our placeholder prefix
                return ciphertext.substr(21); // strlen("placeholder_encrypted_")
            }
        }
    } catch (...) {
        // Decryption failed
    }
    
    return "[Encrypted message - decryption not yet implemented]";
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
    // TODO: Share session key via to-device messages
    return true;
}

bool MatrixCrypto::ProcessKeyShareEvent(const std::string& event)
{
    // TODO: Process incoming room key events
    return true;
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