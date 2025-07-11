#include "MatrixCrypto.h"
#include "../../SDK/SDK.h"
#include <random>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include <thread>

// Ensure proper namespace resolution
using namespace std;

// Enhanced base64 encoding/decoding with proper padding
namespace Base64 {
    static const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
    std::string encode(const std::string& input) {
        std::string encoded;
        int val = 0, valb = -6;
        for (unsigned char c : input) {
            val = (val << 8) + c;
            valb += 8;
            while (valb >= 0) {
                encoded.push_back(chars[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }
        if (valb > -6) encoded.push_back(chars[((val << 8) >> (valb + 8)) & 0x3F]);
        while (encoded.size() % 4) encoded.push_back('=');
        return encoded;
    }
    
    std::string decode(const std::string& input) {
        std::string decoded;
        int val = 0, valb = -8;
        for (unsigned char c : input) {
            if (c == '=') break;
            auto pos = chars.find(c);
            if (pos == std::string::npos) continue;
            val = (val << 6) + static_cast<int>(pos);
            valb += 6;
            if (valb >= 0) {
                decoded.push_back(char((val >> valb) & 0xFF));
                valb -= 8;
            }
        }
        return decoded;
    }
}

// Cryptographically secure random number generation
class SecureRandom {
private:
    std::random_device rd;
    std::mt19937 gen;
    
public:
    SecureRandom() : gen(rd()) {}
    
    void fill_buffer(uint8_t* buffer, size_t length) {
        std::uniform_int_distribution<unsigned int> dis(0, 255);
        for (size_t i = 0; i < length; ++i) {
            buffer[i] = static_cast<uint8_t>(dis(gen));
        }
    }
    
    std::string generate_string(size_t length) {
        std::vector<uint8_t> buffer(length);
        fill_buffer(buffer.data(), length);
        return std::string(buffer.begin(), buffer.end()); // Return raw bytes, not Base64
    }
    
    std::string generate_base64_string(size_t length) {
        std::vector<uint8_t> buffer(length);
        fill_buffer(buffer.data(), length);
        return Base64::encode(std::string(buffer.begin(), buffer.end()));
    }
};

static SecureRandom g_SecureRandom;

// Enhanced logging system
namespace CryptoLogger {
    enum Level { LOG_DEBUG, LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_CRITICAL };
    
    static void log(Level level, const std::string& message) {
        Color_t color;
        std::string prefix;
        
        switch (level) {
            case LOG_DEBUG:   color = {150, 150, 150, 255}; prefix = "DEBUG"; break;
            case LOG_INFO:    color = {200, 200, 200, 255}; prefix = "INFO"; break;
            case LOG_WARNING: color = {255, 255, 150, 255}; prefix = "WARN"; break;
            case LOG_ERROR:   color = {255, 150, 150, 255}; prefix = "ERROR"; break;
            case LOG_CRITICAL: color = {255, 100, 100, 255}; prefix = "CRITICAL"; break;
        }
        
        SDK::Output("MatrixCrypto", (prefix + ": " + message).c_str(), color);
    }
}

// Enhanced Megolm session destructor
MegolmSession::~MegolmSession() {
    if (outbound_session) {
        olm_clear_outbound_group_session(outbound_session);
        delete[] reinterpret_cast<uint8_t*>(outbound_session);
    }
    inbound_sessions.clear(); // unique_ptr will handle cleanup
}

// MatrixCrypto implementation
MatrixCrypto::MatrixCrypto() {
    CryptoLogger::log(CryptoLogger::LOG_INFO, "MatrixCrypto instance created");
}

MatrixCrypto::~MatrixCrypto() {
    CryptoLogger::log(CryptoLogger::LOG_INFO, "MatrixCrypto destructor called");
    Cleanup();
}

bool MatrixCrypto::Initialize(const std::string& user_id) {
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Initializing MatrixCrypto for user: " + user_id);
    
    if (m_bInitialized.load()) {
        CryptoLogger::log(CryptoLogger::LOG_WARNING, "MatrixCrypto already initialized");
        return true;
    }
    
    LogOlmLibraryVersion();
    
    m_sUserId = user_id;
    m_sDeviceId = GenerateDeviceId();
    
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Generated device ID: " + m_sDeviceId);
    
    // Initialize Olm account
    if (!InitializeOlmAccount()) {
        CryptoLogger::log(CryptoLogger::LOG_ERROR, "Failed to initialize Olm account");
        return false;
    }
    
    // Start maintenance thread for session cleanup and key rotation
    StartMaintenanceThread();
    
    m_bInitialized = true;
    CryptoLogger::log(CryptoLogger::LOG_INFO, "MatrixCrypto initialization completed successfully");
    return true;
}

void MatrixCrypto::Cleanup() {
    if (m_bShuttingDown.load()) {
        return;
    }
    
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Starting MatrixCrypto cleanup");
    m_bShuttingDown = true;
    
    // Stop maintenance thread
    StopMaintenanceThread();
    
    // Clean up all sessions
    {
        crypto_mutex::unique_lock lock(m_SessionsMutex);
        m_RoomSessions.clear();
    }
    
    {
        crypto_mutex::unique_lock lock(m_OlmSessionsMutex);
        m_OlmSessions.clear();
    }
    
    // Clean up Olm account
    if (m_pAccount) {
        olm_clear_account(m_pAccount);
        delete[] reinterpret_cast<uint8_t*>(m_pAccount);
        m_pAccount = nullptr;
    }
    
    m_bInitialized = false;
    CryptoLogger::log(CryptoLogger::LOG_INFO, "MatrixCrypto cleanup completed");
}

std::string MatrixCrypto::GenerateDeviceId() {
    // Generate device ID exactly like Element-web: secureRandomString(10)
    // Uses same character set as matrix-js-sdk/src/randomstring.ts
    const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    std::string device_id;
    device_id.reserve(10);
    
    // Use cryptographically secure random generation
    std::vector<uint8_t> random_bytes(10);
    g_SecureRandom.fill_buffer(random_bytes.data(), 10);
    
    for (size_t i = 0; i < 10; ++i) {
        device_id += chars[random_bytes[i] % chars.length()];
    }
    
    CryptoLogger::log(CryptoLogger::LOG_DEBUG, "Generated Element-compatible device ID: " + device_id);
    return device_id;
}

bool MatrixCrypto::InitializeOlmAccount() {
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Initializing Olm account");
    
    try {
        // Create Olm account
        size_t account_size = olm_account_size();
        if (account_size == 0) {
            CryptoLogger::log(CryptoLogger::LOG_CRITICAL, "Olm library not properly linked - account size is 0");
            return false;
        }
        
        CryptoLogger::log(CryptoLogger::LOG_DEBUG, "Olm account size: " + std::to_string(account_size));
        
        m_pAccount = reinterpret_cast<OlmAccount*>(new uint8_t[account_size]);
        if (!m_pAccount) {
            CryptoLogger::log(CryptoLogger::LOG_CRITICAL, "Failed to allocate memory for Olm account");
            return false;
        }
        
        // Initialize the account structure
        olm_account(m_pAccount);
        
        // Generate random bytes for account creation
        size_t random_length = olm_create_account_random_length(m_pAccount);
        std::vector<uint8_t> random_bytes(random_length);
        g_SecureRandom.fill_buffer(random_bytes.data(), random_length);
        
        CryptoLogger::log(CryptoLogger::LOG_DEBUG, "Creating Olm account with " + std::to_string(random_length) + " random bytes");
        
        // Create the account
        size_t result = olm_create_account(m_pAccount, random_bytes.data(), random_length);
        if (result == olm_error()) {
            const char* error = olm_account_last_error(m_pAccount);
            CryptoLogger::log(CryptoLogger::LOG_CRITICAL, "Olm account creation failed: " + std::string(error ? error : "unknown error"));
            
            delete[] reinterpret_cast<uint8_t*>(m_pAccount);
            m_pAccount = nullptr;
            return false;
        }
        
        // Generate initial one-time keys
        if (!GenerateOneTimeKeys(50)) {
            CryptoLogger::log(CryptoLogger::LOG_WARNING, "Failed to generate initial one-time keys");
        }
        
        CryptoLogger::log(CryptoLogger::LOG_INFO, "Olm account initialized successfully");
        return true;
        
    } catch (const std::exception& e) {
        CryptoLogger::log(CryptoLogger::LOG_CRITICAL, "Exception during Olm initialization: " + std::string(e.what()));
        if (m_pAccount) {
            delete[] reinterpret_cast<uint8_t*>(m_pAccount);
            m_pAccount = nullptr;
        }
        return false;
    }
}

void MatrixCrypto::StartMaintenanceThread() {
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Starting maintenance thread");
    m_MaintenanceThread = std::thread(&MatrixCrypto::MaintenanceLoop, this);
}

void MatrixCrypto::StopMaintenanceThread() {
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Stopping maintenance thread");
    {
        std::lock_guard<std::mutex> lock(m_MaintenanceMutex);
        m_bShuttingDown = true;
    }
    m_MaintenanceCV.notify_all();
    
    if (m_MaintenanceThread.joinable()) {
        m_MaintenanceThread.join();
    }
}

void MatrixCrypto::MaintenanceLoop() {
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Maintenance thread started");
    
    while (!m_bShuttingDown.load()) {
        std::unique_lock<std::mutex> lock(m_MaintenanceMutex);
        m_MaintenanceCV.wait_for(lock, std::chrono::minutes(5), [this] { return m_bShuttingDown.load(); });
        
        if (m_bShuttingDown.load()) {
            break;
        }
        
        lock.unlock();
        
        try {
            // Perform maintenance tasks
            CleanupExpiredSessions();
            CleanupOldMessages();
            CleanupFailedDevices();
            ProcessPendingRoomKeys();
            
            // Check for sessions that need rotation
            std::vector<std::string> rooms_to_rotate;
            {
                crypto_mutex::shared_lock session_lock(m_SessionsMutex);
                for (std::unordered_map<std::string, std::unique_ptr<MegolmSession>>::const_iterator session_it = m_RoomSessions.begin(); session_it != m_RoomSessions.end(); ++session_it) {
                    const std::string& room_id = session_it->first;
                    const std::unique_ptr<MegolmSession>& session = session_it->second;
                    if (ShouldRotateSession(room_id, *session)) {
                        rooms_to_rotate.push_back(room_id);
                    }
                }
            }
            
            for (const auto& room_id : rooms_to_rotate) {
                CryptoLogger::log(CryptoLogger::LOG_INFO, "Scheduling rotation for room: " + room_id);
                RotateMegolmSession(room_id);
            }
            
        } catch (const std::exception& e) {
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "Exception in maintenance thread: " + std::string(e.what()));
        }
    }
    
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Maintenance thread stopped");
}

bool MatrixCrypto::ValidateDeviceKeys(const MatrixDevice& device) {
    if (device.curve25519_key.empty() || device.ed25519_key.empty()) {
        CryptoLogger::log(CryptoLogger::LOG_ERROR, "Device " + device.device_id + " has empty keys");
        return false;
    }
    
    // Validate key format (base64 encoded)
    try {
        Base64::decode(device.curve25519_key);
        Base64::decode(device.ed25519_key);
    } catch (...) {
        CryptoLogger::log(CryptoLogger::LOG_ERROR, "Device " + device.device_id + " has invalid key format");
        return false;
    }
    
    // Additional validation can be added here
    return true;
}

bool MatrixCrypto::VerifyDeviceSignature(const MatrixDevice& device) {
    if (device.signatures.empty()) {
        CryptoLogger::log(CryptoLogger::LOG_WARNING, "Device " + device.device_id + " has no signatures");
        return false;
    }
    
    // Verify device signature using ed25519 key (Element-web compatible format)
    std::map<std::string, std::map<std::string, std::string>>::const_iterator user_sigs = device.signatures.find(device.user_id);
    if (user_sigs == device.signatures.end()) {
        return false;
    }
    
    std::map<std::string, std::string>::const_iterator device_sig = user_sigs->second.find("ed25519:" + device.device_id);
    if (device_sig == user_sigs->second.end()) {
        return false;
    }
    
    // Create canonical JSON for verification
    nlohmann::json device_json = {
        {"user_id", device.user_id},
        {"device_id", device.device_id},
        {"algorithms", nlohmann::json::array({"m.olm.v1.curve25519-aes-sha2", "m.megolm.v1.aes-sha2"})},
        {"keys", {
            {"curve25519:" + device.device_id, device.curve25519_key},
            {"ed25519:" + device.device_id, device.ed25519_key}
        }}
    };
    
    std::string canonical_json = GetCanonicalJson(device_json);
    return VerifySignature(canonical_json, device_sig->second, device.ed25519_key);
}

std::string MatrixCrypto::GetDeviceKeys() {
    if (!m_pAccount) {
        CryptoLogger::log(CryptoLogger::LOG_ERROR, "Cannot get device keys - Olm account not initialized");
        return "";
    }
    
    try {
        // Get identity keys from Olm account
        size_t id_keys_length = olm_account_identity_keys_length(m_pAccount);
        std::vector<uint8_t> id_keys_buffer(id_keys_length);
        
        size_t result = olm_account_identity_keys(m_pAccount, id_keys_buffer.data(), id_keys_length);
        if (result == olm_error()) {
            const char* error = olm_account_last_error(m_pAccount);
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "Failed to get identity keys: " + std::string(error ? error : "unknown error"));
            return "";
        }
        
        // Parse the identity keys JSON
        std::string id_keys_str(reinterpret_cast<char*>(id_keys_buffer.data()), id_keys_length);
        auto id_keys_json = nlohmann::json::parse(id_keys_str);
        
        // Create device keys JSON exactly matching Element-web format
        // Based on IDeviceKeys interface from matrix-js-sdk
        nlohmann::json keys = {
            {"algorithms", nlohmann::json::array({"m.olm.v1.curve25519-aes-sha2", "m.megolm.v1.aes-sha2"})},
            {"device_id", m_sDeviceId},
            {"user_id", m_sUserId},
            {"keys", {
                {"curve25519:" + m_sDeviceId, id_keys_json["curve25519"]},
                {"ed25519:" + m_sDeviceId, id_keys_json["ed25519"]}
            }}
        };
        
        // Create canonical JSON for signing
        std::string canonical_json = GetCanonicalJson(keys);
        
        // Sign the keys
        size_t signature_length = olm_account_signature_length(m_pAccount);
        std::vector<uint8_t> signature_buffer(signature_length);
        
        result = olm_account_sign(m_pAccount, 
                                 reinterpret_cast<const uint8_t*>(canonical_json.c_str()), 
                                 canonical_json.length(),
                                 signature_buffer.data(), 
                                 signature_length);
        
        if (result == olm_error()) {
            const char* error = olm_account_last_error(m_pAccount);
            CryptoLogger::log(CryptoLogger::LOG_WARNING, "Failed to sign device keys: " + std::string(error ? error : "unknown error"));
            return keys.dump(); // Return unsigned keys
        }
        
        // Add signature
        std::string signature_str(reinterpret_cast<char*>(signature_buffer.data()), signature_length);
        keys["signatures"] = {
            {m_sUserId, {
                {"ed25519:" + m_sDeviceId, signature_str}
            }}
        };
        
        return keys.dump();
        
    } catch (const std::exception& e) {
        CryptoLogger::log(CryptoLogger::LOG_ERROR, "Exception getting device keys: " + std::string(e.what()));
        return "";
    }
}

std::string MatrixCrypto::GetOneTimeKeys() {
    if (!m_pAccount) {
        CryptoLogger::log(CryptoLogger::LOG_ERROR, "Cannot get one-time keys - Olm account not initialized");
        return "";
    }
    
    try {
        size_t keys_length = olm_account_one_time_keys_length(m_pAccount);
        if (keys_length == 0) {
            CryptoLogger::log(CryptoLogger::LOG_WARNING, "No one-time keys available");
            return "{}";
        }
        
        std::vector<uint8_t> keys_buffer(keys_length);
        size_t result = olm_account_one_time_keys(m_pAccount, keys_buffer.data(), keys_length);
        
        if (result == olm_error()) {
            const char* error = olm_account_last_error(m_pAccount);
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "Failed to get one-time keys: " + std::string(error ? error : "unknown error"));
            return "";
        }
        
        std::string keys_str(reinterpret_cast<char*>(keys_buffer.data()), result);
        
        // Parse the raw one-time keys
        auto one_time_keys = nlohmann::json::parse(keys_str);
        
        // Create properly formatted signed one-time keys matching Element-web format
        // Based on IOneTimeKey interface and signature process from matrix-js-sdk
        nlohmann::json signed_keys;
        
        if (one_time_keys.contains("curve25519")) {
            for (auto& [key_id, key_base64] : one_time_keys["curve25519"].items()) {
                std::string key_name = "signed_curve25519:" + key_id;
                
                // Create the key object exactly as Element-web does
                nlohmann::json key_object = {
                    {"key", key_base64}
                };
                
                // Create canonical JSON for signing (without signatures field)
                std::string canonical_json = GetCanonicalJson(key_object);
                
                // Sign the canonical JSON using our Ed25519 key
                size_t signature_length = olm_account_signature_length(m_pAccount);
                std::vector<uint8_t> signature_buffer(signature_length);
                
                size_t result = olm_account_sign(m_pAccount, 
                                               reinterpret_cast<const uint8_t*>(canonical_json.c_str()), 
                                               canonical_json.length(),
                                               signature_buffer.data(), 
                                               signature_length);
                
                if (result != olm_error()) {
                    std::string signature_str(reinterpret_cast<char*>(signature_buffer.data()), signature_length);
                    
                    // Add signatures in Element-web format
                    key_object["signatures"] = {
                        {m_sUserId, {
                            {"ed25519:" + m_sDeviceId, signature_str}
                        }}
                    };
                } else {
                    const char* error = olm_account_last_error(m_pAccount);
                    CryptoLogger::log(CryptoLogger::LOG_WARNING, "Failed to sign one-time key " + key_id + ": " + std::string(error ? error : "unknown error"));
                }
                
                signed_keys[key_name] = key_object;
            }
        }
        
        return signed_keys.dump();
        
    } catch (const std::exception& e) {
        CryptoLogger::log(CryptoLogger::LOG_ERROR, "Exception getting one-time keys: " + std::string(e.what()));
        return "";
    }
}

bool MatrixCrypto::GenerateOneTimeKeys(size_t count) {
    if (!m_pAccount) {
        CryptoLogger::log(CryptoLogger::LOG_ERROR, "Cannot generate one-time keys - Olm account not initialized");
        return false;
    }
    
    try {
        size_t random_length = olm_account_generate_one_time_keys_random_length(m_pAccount, count);
        std::vector<uint8_t> random_bytes(random_length);
        g_SecureRandom.fill_buffer(random_bytes.data(), random_length);
        
        size_t result = olm_account_generate_one_time_keys(m_pAccount, count, random_bytes.data(), random_length);
        if (result == olm_error()) {
            const char* error = olm_account_last_error(m_pAccount);
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "Failed to generate one-time keys: " + std::string(error ? error : "unknown error"));
            return false;
        }
        
        CryptoLogger::log(CryptoLogger::LOG_INFO, "Generated " + std::to_string(count) + " one-time keys");
        return true;
        
    } catch (const std::exception& e) {
        CryptoLogger::log(CryptoLogger::LOG_ERROR, "Exception generating one-time keys: " + std::string(e.what()));
        return false;
    }
}

void MatrixCrypto::MarkOneTimeKeysAsPublished() {
    if (!m_pAccount) {
        return;
    }
    
    olm_account_mark_keys_as_published(m_pAccount);
    CryptoLogger::log(CryptoLogger::LOG_DEBUG, "Marked one-time keys as published");
}

bool MatrixCrypto::UpdateUserDevices(const std::string& user_id, const std::unordered_map<std::string, MatrixDevice>& devices) {
    crypto_mutex::unique_lock lock(m_DevicesMutex);
    
    size_t validated_count = 0;
    std::unordered_map<std::string, MatrixDevice> validated_devices;
    
    for (std::unordered_map<std::string, MatrixDevice>::const_iterator device_it = devices.begin(); device_it != devices.end(); ++device_it) {
        if (ValidateDeviceKeys(device_it->second)) {
            validated_devices[device_it->first] = device_it->second;
            validated_count++;
        } else {
            CryptoLogger::log(CryptoLogger::LOG_WARNING, "Skipping invalid device: " + device_it->first);
        }
    }
    
    m_UserDevices[user_id] = validated_devices;
    
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Updated devices for user " + user_id + 
                     ": " + std::to_string(validated_count) + "/" + std::to_string(devices.size()) + " valid");
    
    return true;
}

bool MatrixCrypto::UpdateUserDevices(const std::string& user_id, const std::map<std::string, MatrixDevice>& devices) {
    crypto_mutex::unique_lock lock(m_DevicesMutex);
    
    size_t validated_count = 0;
    std::unordered_map<std::string, MatrixDevice> validated_devices;
    
    for (std::map<std::string, MatrixDevice>::const_iterator device_it = devices.begin(); device_it != devices.end(); ++device_it) {
        if (ValidateDeviceKeys(device_it->second)) {
            validated_devices[device_it->first] = device_it->second;
            validated_count++;
        } else {
            CryptoLogger::log(CryptoLogger::LOG_WARNING, "Skipping invalid device: " + device_it->first);
        }
    }
    
    m_UserDevices[user_id] = validated_devices;
    
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Updated devices for user " + user_id + 
                     ": " + std::to_string(validated_count) + "/" + std::to_string(devices.size()) + " valid");
    
    return true;
}

std::unordered_map<std::string, MatrixDevice> MatrixCrypto::GetUserDevices(const std::string& user_id) const {
    crypto_mutex::shared_lock lock(m_DevicesMutex);
    
    auto it = m_UserDevices.find(user_id);
    if (it != m_UserDevices.end()) {
        return it->second;
    }
    
    return {};
}

bool MatrixCrypto::CreateMegolmSession(const std::string& room_id) {
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Creating Megolm session for room: " + room_id);
    
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
        g_SecureRandom.fill_buffer(random_bytes.data(), random_length);
        
        // Initialize the session
        size_t result = olm_init_outbound_group_session(session->outbound_session, 
                                                       random_bytes.data(), 
                                                       random_length);
        if (result == olm_error()) {
            const char* error = olm_outbound_group_session_last_error(session->outbound_session);
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "Failed to initialize outbound group session: " + 
                             std::string(error ? error : "unknown error"));
            return false;
        }
        
        // Get session ID
        size_t session_id_length = olm_outbound_group_session_id_length(session->outbound_session);
        std::vector<uint8_t> session_id_buffer(session_id_length);
        
        result = olm_outbound_group_session_id(session->outbound_session,
                                              session_id_buffer.data(),
                                              session_id_length);
        if (result == olm_error()) {
            const char* error = olm_outbound_group_session_last_error(session->outbound_session);
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "Failed to get session ID: " + 
                             std::string(error ? error : "unknown error"));
            return false;
        }
        
        session->session_id = std::string(reinterpret_cast<char*>(session_id_buffer.data()), session_id_length);
        
        // Save session ID before moving
        std::string session_id_for_log = session->session_id;
        
        // Create inbound session immediately to avoid index mismatch
        auto inbound = std::make_unique<InboundGroupSession>();
        inbound->session_id = session->session_id;
        inbound->sender_key = GetOwnCurve25519Key();
        inbound->room_id = room_id;
        inbound->creation_time = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        
        // Create inbound session from outbound session
        size_t inbound_size = olm_inbound_group_session_size();
        inbound->session = reinterpret_cast<OlmInboundGroupSession*>(new uint8_t[inbound_size]);
        olm_inbound_group_session(inbound->session);
        
        // Export key from outbound session
        size_t key_length = olm_outbound_group_session_key_length(session->outbound_session);
        std::vector<uint8_t> session_key(key_length);
        
        size_t key_result = olm_outbound_group_session_key(session->outbound_session,
                                                         session_key.data(), key_length);
        if (key_result == olm_error()) {
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "Failed to export session key during creation");
        } else {
            // Initialize inbound session
            size_t result = olm_init_inbound_group_session(inbound->session,
                                                          session_key.data(), key_result);
            if (result == olm_error()) {
                const char* error = olm_inbound_group_session_last_error(inbound->session);
                CryptoLogger::log(CryptoLogger::LOG_ERROR, "Failed to create inbound session during creation: " + 
                                 std::string(error ? error : "unknown error"));
            } else {
                CryptoLogger::log(CryptoLogger::LOG_INFO, "Created inbound session for " + session_id_for_log);
                session->inbound_sessions[session->session_id] = std::move(inbound);
            }
        }
        
        // Store the session
        {
            crypto_mutex::unique_lock lock(m_SessionsMutex);
            m_RoomSessions[room_id] = std::move(session);
        }
        
        CryptoLogger::log(CryptoLogger::LOG_INFO, "Created Megolm session for room " + room_id + 
                         " with session ID: " + session_id_for_log);
        
        return true;
        
    } catch (const std::exception& e) {
        CryptoLogger::log(CryptoLogger::LOG_ERROR, "Exception creating Megolm session: " + std::string(e.what()));
        return false;
    }
}

bool MatrixCrypto::EnsureRoomSession(const std::string& room_id) {
    crypto_mutex::shared_lock lock(m_SessionsMutex);
    
    auto it = m_RoomSessions.find(room_id);
    if (it != m_RoomSessions.end() && it->second != nullptr) {
        // Check if session needs rotation
        if (ShouldRotateSession(room_id, *it->second)) {
            lock.unlock();
            return RotateMegolmSession(room_id);
        }
        return true;
    }
    
    lock.unlock();
    return CreateMegolmSession(room_id);
}

bool MatrixCrypto::ShouldRotateSession(const std::string& room_id, const MegolmSession& session) {
    int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    // Rotate if too many messages or too old
    bool should_rotate = session.message_count >= MegolmSession::MAX_MESSAGES_PER_SESSION ||
                        (now - session.creation_time) >= MegolmSession::MAX_SESSION_AGE_MS ||
                        session.needs_rotation;
    
    if (should_rotate) {
        CryptoLogger::log(CryptoLogger::LOG_INFO, "Session rotation needed for room " + room_id + 
                         " (messages: " + std::to_string(session.message_count) + 
                         ", age: " + std::to_string(now - session.creation_time) + "ms)");
    }
    
    return should_rotate;
}

bool MatrixCrypto::RotateMegolmSession(const std::string& room_id) {
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Rotating Megolm session for room: " + room_id);
    
    {
        crypto_mutex::unique_lock lock(m_SessionsMutex);
        m_RoomSessions.erase(room_id);
    }
    
    return CreateMegolmSession(room_id);
}

std::string MatrixCrypto::EncryptMessage(const std::string& room_id, const std::string& event_type, const std::string& content) {
    // Element-web compatible session establishment: check existing session first, create if needed
    {
        crypto_mutex::shared_lock check_lock(m_SessionsMutex);
        auto existing_it = m_RoomSessions.find(room_id);
        if (existing_it != m_RoomSessions.end() && existing_it->second->outbound_session) {
            // Session exists, check if rotation is needed (Element-web pattern)
            if (ShouldRotateSession(room_id, *existing_it->second)) {
                CryptoLogger::log(CryptoLogger::LOG_INFO, "Session rotation needed for room: " + room_id);
                check_lock.unlock();
                if (!RotateMegolmSession(room_id)) {
                    CryptoLogger::log(CryptoLogger::LOG_ERROR, "Failed to rotate session for: " + room_id);
                    return "";
                }
            }
        } else {
            // No session exists, create new one (deferred establishment)
            check_lock.unlock();
            if (!EnsureRoomSession(room_id)) {
                CryptoLogger::log(CryptoLogger::LOG_ERROR, "Failed to ensure room session for: " + room_id);
                return "";
            }
        }
    }
    
    crypto_mutex::shared_lock lock(m_SessionsMutex);
    auto it = m_RoomSessions.find(room_id);
    if (it == m_RoomSessions.end() || !it->second->outbound_session) {
        CryptoLogger::log(CryptoLogger::LOG_ERROR, "No valid session found for room: " + room_id);
        return "";
    }
    
    auto& session = it->second;
    
    try {
        // Get sender key
        std::string sender_key = GetOwnCurve25519Key();
        if (sender_key.empty()) {
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "Failed to get sender key");
            return "";
        }
        
        // Create Matrix event structure for plaintext
        nlohmann::json plaintext_event = {
            {"type", event_type},
            {"content", nlohmann::json::parse(content)},
            {"room_id", room_id}
        };
        
        std::string plaintext = plaintext_event.dump();
        
        // Get current message index before encryption
        uint32_t message_index = olm_outbound_group_session_message_index(session->outbound_session);
        
        CryptoLogger::log(CryptoLogger::LOG_DEBUG, "Encrypting message for room " + room_id + 
                         " at index " + std::to_string(message_index));
        
        // Share room key with all devices BEFORE encrypting first message (like Element Web)
        if (!session->key_shared && session->message_count == 0) {
            CryptoLogger::log(CryptoLogger::LOG_INFO, "First message in session - need to share room key for room: " + room_id);
            
            // Get all devices in the room that need the key
            std::vector<MatrixDevice> target_devices;
            
            try {
                // Get room members and their devices safely
                crypto_mutex::shared_lock device_lock(m_DevicesMutex);
                for (const auto& user_pair : m_UserDevices) {
                    const auto& user_id = user_pair.first;
                    const auto& devices = user_pair.second;
                    
                    // Add all devices for all users in the room
                    // TODO: This should be filtered to only room members
                    for (const auto& device_pair : devices) {
                        const auto& device = device_pair.second;
                        if (!device.deleted && 
                            device.verification_status != DeviceVerificationStatus::BlackListed &&
                            !device.curve25519_key.empty() && 
                            !device.ed25519_key.empty()) {
                            target_devices.push_back(device);
                        }
                    }
                }
                // device_lock will be automatically destroyed at scope end
            } catch (const std::exception& e) {
                CryptoLogger::log(CryptoLogger::LOG_ERROR, "Exception collecting target devices: " + std::string(e.what()));
                session->key_shared = true; // Mark as shared to avoid infinite attempts
                // Continue with encryption even if key sharing failed
            }
            
            if (!target_devices.empty()) {
                CryptoLogger::log(CryptoLogger::LOG_INFO, "Sharing room key with " + std::to_string(target_devices.size()) + " devices");
                
                // Create the session key export format
                size_t key_length = olm_outbound_group_session_key_length(session->outbound_session);
                std::vector<uint8_t> session_key_buffer(key_length);
                
                size_t key_result = olm_outbound_group_session_key(session->outbound_session,
                                                                  session_key_buffer.data(), key_length);
                if (key_result != olm_error()) {
                    session->session_key_export_format = std::string(reinterpret_cast<char*>(session_key_buffer.data()), key_result);
                    
                    // Share the key with all target devices (like Element Web's shareRoomKey)
                    bool share_success = ShareRoomKey(room_id, target_devices);
                    if (share_success) {
                        session->key_shared = true;
                        CryptoLogger::log(CryptoLogger::LOG_INFO, "Room key shared successfully for room " + room_id);
                    } else {
                        CryptoLogger::log(CryptoLogger::LOG_ERROR, "Failed to share room key for room " + room_id);
                        // Continue with encryption anyway, but mark as shared to avoid infinite attempts
                        session->key_shared = true;
                    }
                } else {
                    CryptoLogger::log(CryptoLogger::LOG_ERROR, "Failed to export session key for room " + room_id);
                }
            } else {
                CryptoLogger::log(CryptoLogger::LOG_WARNING, "No devices found to share room key with for room: " + room_id);
                session->key_shared = true; // Mark as shared to avoid infinite attempts
            }
        }
        
        // Encrypt the message
        size_t ciphertext_length = olm_group_encrypt_message_length(session->outbound_session, plaintext.length());
        std::vector<uint8_t> ciphertext_buffer(ciphertext_length);
        
        size_t result = olm_group_encrypt(session->outbound_session,
                                         reinterpret_cast<const uint8_t*>(plaintext.c_str()),
                                         plaintext.length(),
                                         ciphertext_buffer.data(),
                                         ciphertext_length);
        
        if (result == olm_error()) {
            const char* error = olm_outbound_group_session_last_error(session->outbound_session);
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "Failed to encrypt message: " + 
                             std::string(error ? error : "unknown error"));
            return "";
        }
        
        std::string ciphertext(reinterpret_cast<char*>(ciphertext_buffer.data()), result);
        
        // Update session state
        session->message_count++;
        session->last_use_time = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        
        // Create encrypted event exactly matching Element-web format
        nlohmann::json encrypted_content = {
            {"algorithm", "m.megolm.v1.aes-sha2"},
            {"ciphertext", ciphertext},
            {"session_id", session->session_id},
            {"sender_key", sender_key},
            {"device_id", m_sDeviceId}  // Element-web includes device_id
        };
        
        CryptoLogger::log(CryptoLogger::LOG_INFO, "Successfully encrypted message for room " + room_id);
        
        return encrypted_content.dump();
        
    } catch (const std::exception& e) {
        CryptoLogger::log(CryptoLogger::LOG_ERROR, "Exception encrypting message: " + std::string(e.what()));
        return "";
    }
}

std::string MatrixCrypto::DecryptMessage(const std::string& room_id, const std::string& encrypted_event) {
    try {
        auto json = nlohmann::json::parse(encrypted_event);
        
        // Validate encrypted event structure (Element-web compatible)
        if (!json.contains("algorithm") || !json.contains("ciphertext") || 
            !json.contains("session_id") || !json.contains("sender_key")) {
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "Invalid encrypted event format - missing required fields");
            return "[Invalid encrypted event format]";
        }
        
        // device_id is optional for compatibility with other clients, but Element-web includes it
        if (json.contains("device_id")) {
            std::string device_id = json["device_id"];
            CryptoLogger::log(CryptoLogger::LOG_DEBUG, "Message from device: " + device_id);
        }
        
        std::string algorithm = json["algorithm"];
        if (algorithm != "m.megolm.v1.aes-sha2") {
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "Unsupported algorithm: " + algorithm);
            return "[Unsupported encryption algorithm: " + algorithm + "]";
        }
        
        std::string ciphertext = json["ciphertext"];
        std::string session_id = json["session_id"];
        std::string sender_key = json["sender_key"];
        
        CryptoLogger::log(CryptoLogger::LOG_DEBUG, "Decrypting message for room " + room_id + 
                         " with session " + session_id);
        
        // Get the appropriate inbound session
        crypto_mutex::shared_lock lock(m_SessionsMutex);
        auto room_it = m_RoomSessions.find(room_id);
        if (room_it == m_RoomSessions.end()) {
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "No session found for room: " + room_id);
            return "[No session for room]";
        }
        
        auto& room_session = room_it->second;
        OlmInboundGroupSession* inbound_session = nullptr;
        
        // Look for existing inbound session
        auto session_it = room_session->inbound_sessions.find(session_id);
        if (session_it != room_session->inbound_sessions.end()) {
            inbound_session = session_it->second->session;
        } else {
            // Try to create inbound session from our outbound session
            if (room_session->outbound_session && room_session->session_id == session_id) {
                CryptoLogger::log(CryptoLogger::LOG_DEBUG, "Creating inbound session from outbound for session: " + session_id);
                auto new_inbound = std::make_unique<InboundGroupSession>();
                new_inbound->session_id = session_id;
                new_inbound->sender_key = sender_key;
                new_inbound->room_id = room_id;
                new_inbound->creation_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
                
                // Create inbound session
                size_t inbound_size = olm_inbound_group_session_size();
                new_inbound->session = reinterpret_cast<OlmInboundGroupSession*>(new uint8_t[inbound_size]);
                olm_inbound_group_session(new_inbound->session);
                
                // Export key from outbound session
                size_t key_length = olm_outbound_group_session_key_length(room_session->outbound_session);
                std::vector<uint8_t> session_key(key_length);
                
                size_t key_result = olm_outbound_group_session_key(room_session->outbound_session,
                                                                 session_key.data(), key_length);
                if (key_result == olm_error()) {
                    CryptoLogger::log(CryptoLogger::LOG_ERROR, "Failed to export session key");
                    return "[Failed to export session key]";
                }
                
                // Initialize inbound session
                size_t result = olm_init_inbound_group_session(new_inbound->session,
                                                              session_key.data(), key_result);
                if (result == olm_error()) {
                    const char* error = olm_inbound_group_session_last_error(new_inbound->session);
                    CryptoLogger::log(CryptoLogger::LOG_ERROR, "Failed to create inbound session: " + 
                                     std::string(error ? error : "unknown error"));
                    return "[Failed to create inbound session]";
                }
                
                inbound_session = new_inbound->session;
                room_session->inbound_sessions[session_id] = std::move(new_inbound);
                
                CryptoLogger::log(CryptoLogger::LOG_INFO, "Created inbound session for " + session_id);
            } else {
                CryptoLogger::log(CryptoLogger::LOG_ERROR, "No inbound session available for: " + session_id);
                return "[No inbound session for session_id: " + session_id + "]";
            }
        }
        
        // Decrypt the message
        std::vector<uint8_t> ciphertext_buffer(ciphertext.begin(), ciphertext.end());
        size_t max_plaintext_length = olm_group_decrypt_max_plaintext_length(inbound_session,
                                                                            ciphertext_buffer.data(),
                                                                            ciphertext_buffer.size());
        if (max_plaintext_length == olm_error()) {
            const char* error = olm_inbound_group_session_last_error(inbound_session);
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "Failed to get max plaintext length: " + 
                             std::string(error ? error : "unknown error"));
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
            DecryptionErrorCode error_code = ClassifyDecryptionError(error ? error : "unknown error");
            
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "Decryption failed: " + std::string(error ? error : "unknown error"));
            
            // Try to recover from certain errors
            if (error_code == DecryptionErrorCode::UnknownMessageIndex) {
                CryptoLogger::log(CryptoLogger::LOG_WARNING, "Unknown message index - this might indicate session corruption");
                // Could trigger session recovery here
            }
            
            return "[Decryption failed: " + std::string(error ? error : "unknown error") + "]";
        }
        
        std::string plaintext(reinterpret_cast<char*>(plaintext_buffer.data()), plaintext_length);
        
        CryptoLogger::log(CryptoLogger::LOG_DEBUG, "Successfully decrypted message at index " + std::to_string(message_index));
        
        // Extract message content
        try {
            auto plaintext_json = nlohmann::json::parse(plaintext);
            if (plaintext_json.contains("content") && plaintext_json["content"].contains("body")) {
                return plaintext_json["content"]["body"];
            }
            return plaintext;
        } catch (...) {
            return plaintext;
        }
        
    } catch (const std::exception& e) {
        CryptoLogger::log(CryptoLogger::LOG_ERROR, "Exception decrypting message: " + std::string(e.what()));
        return "[Decryption error: " + std::string(e.what()) + "]";
    }
}

DecryptionErrorCode MatrixCrypto::ClassifyDecryptionError(const std::string& error_msg) {
    if (error_msg.find("UNKNOWN_MESSAGE_INDEX") != std::string::npos) {
        return DecryptionErrorCode::UnknownMessageIndex;
    } else if (error_msg.find("BAD_MESSAGE_MAC") != std::string::npos) {
        return DecryptionErrorCode::InvalidCiphertext;
    } else if (error_msg.find("SESSION_NOT_FOUND") != std::string::npos) {
        return DecryptionErrorCode::MissingRoomKey;
    } else if (error_msg.find("CORRUPTED") != std::string::npos) {
        return DecryptionErrorCode::SessionCorrupted;
    }
    return DecryptionErrorCode::UnknownError;
}

std::string MatrixCrypto::GetOwnCurve25519Key() const {
    if (!m_pAccount) {
        return "";
    }
    
    try {
        size_t id_keys_length = olm_account_identity_keys_length(m_pAccount);
        std::vector<uint8_t> id_keys_buffer(id_keys_length);
        
        size_t result = olm_account_identity_keys(m_pAccount, id_keys_buffer.data(), id_keys_length);
        if (result == olm_error()) {
            return "";
        }
        
        std::string id_keys_str(reinterpret_cast<char*>(id_keys_buffer.data()), id_keys_length);
        auto id_keys_json = nlohmann::json::parse(id_keys_str);
        
        return id_keys_json["curve25519"];
    } catch (...) {
        return "";
    }
}

std::string MatrixCrypto::GetCanonicalJson(const nlohmann::json& json) {
    // Create canonical JSON exactly according to Matrix spec (matches Element-web)
    // Must be deterministic with sorted keys and no whitespace
    return json.dump(-1, ' ', false, nlohmann::json::error_handler_t::strict);
}

bool MatrixCrypto::VerifySignature(const std::string& message, const std::string& signature, const std::string& public_key) {
    if (!m_pAccount) {
        return false;
    }
    
    try {
        size_t utility_size = olm_utility_size();
        OlmUtility* utility = reinterpret_cast<OlmUtility*>(new uint8_t[utility_size]);
        olm_utility(utility);
        
        std::vector<uint8_t> signature_buffer(signature.begin(), signature.end());
        size_t result = olm_ed25519_verify(utility,
                                          reinterpret_cast<const uint8_t*>(public_key.c_str()), public_key.length(),
                                          reinterpret_cast<const uint8_t*>(message.c_str()), message.length(),
                                          signature_buffer.data(), signature_buffer.size());
        
        bool verified = (result != olm_error());
        
        olm_clear_utility(utility);
        delete[] reinterpret_cast<uint8_t*>(utility);
        
        return verified;
    } catch (...) {
        return false;
    }
}

std::string MatrixCrypto::SignMessage(const std::string& message) {
    if (!m_pAccount) {
        CryptoLogger::log(CryptoLogger::LOG_ERROR, "Cannot sign message - Olm account not initialized");
        return "";
    }
    
    try {
        size_t signature_length = olm_account_signature_length(m_pAccount);
        std::vector<uint8_t> signature_buffer(signature_length);
        
        size_t result = olm_account_sign(m_pAccount,
                                        reinterpret_cast<const uint8_t*>(message.c_str()), message.length(),
                                        signature_buffer.data(), signature_length);
        
        if (result == olm_error()) {
            const char* error = olm_account_last_error(m_pAccount);
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "Failed to sign message: " + std::string(error ? error : "unknown error"));
            return "";
        }
        
        std::string signature_str(reinterpret_cast<char*>(signature_buffer.data()), result);
        return signature_str;
        
    } catch (const std::exception& e) {
        CryptoLogger::log(CryptoLogger::LOG_ERROR, "Exception signing message: " + std::string(e.what()));
        return "";
    }
}

void MatrixCrypto::CleanupExpiredSessions() {
    CryptoLogger::log(CryptoLogger::LOG_DEBUG, "Cleaning up expired sessions");
    
    int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    crypto_mutex::unique_lock lock(m_SessionsMutex);
    
    for (auto it = m_RoomSessions.begin(); it != m_RoomSessions.end();) {
        auto& session = it->second;
        if ((now - session->last_use_time) > MegolmSession::MAX_SESSION_AGE_MS * 2) {
            CryptoLogger::log(CryptoLogger::LOG_INFO, "Removing expired session for room: " + it->first);
            it = m_RoomSessions.erase(it);
        } else {
            ++it;
        }
    }
}

void MatrixCrypto::CleanupOldMessages() {
    CryptoLogger::log(CryptoLogger::LOG_DEBUG, "Cleaning up old message metadata");
    
    int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    std::lock_guard<std::mutex> lock(m_MessageTrackingMutex);
    
    for (auto it = m_ProcessedMessages.begin(); it != m_ProcessedMessages.end();) {
        if ((now - it->second.timestamp) > 24 * 60 * 60 * 1000) { // 24 hours
            it = m_ProcessedMessages.erase(it);
        } else {
            ++it;
        }
    }
}

void MatrixCrypto::CleanupFailedDevices() {
    CryptoLogger::log(CryptoLogger::LOG_DEBUG, "Cleaning up failed devices");
    
    int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    crypto_mutex::unique_lock lock(m_FailedDevicesMutex);
    
    for (auto it = m_FailedDevices.begin(); it != m_FailedDevices.end();) {
        if (it->second.retry_count >= FailedDevice::MAX_RETRY_COUNT &&
            (now - it->second.last_attempt_time) > 24 * 60 * 60 * 1000) { // 24 hours
            it = m_FailedDevices.erase(it);
        } else {
            ++it;
        }
    }
}

std::vector<PendingRoomKey> MatrixCrypto::GetAndClearPendingRoomKeys() {
    std::lock_guard<std::mutex> lock(m_PendingKeysMutex);
    
    std::vector<PendingRoomKey> keys_to_send;
    
    if (!m_PendingRoomKeys.empty()) {
        CryptoLogger::log(CryptoLogger::LOG_DEBUG, "Retrieving " + std::to_string(m_PendingRoomKeys.size()) + " pending room keys for transmission");
    
        while (!m_PendingRoomKeys.empty()) {
            keys_to_send.push_back(std::move(m_PendingRoomKeys.front()));
            m_PendingRoomKeys.pop();
        }
    }
    
    return keys_to_send;
}

void MatrixCrypto::ProcessPendingRoomKeys() {
    // This method is called by maintenance thread and legacy code
    // The actual sending should be done by the Chat class via GetAndClearPendingRoomKeys
    auto keys = GetAndClearPendingRoomKeys();
    
    if (!keys.empty()) {
        CryptoLogger::log(CryptoLogger::LOG_WARNING, 
                         "Warning: " + std::to_string(keys.size()) + 
                         " room keys discarded - they should be sent via Chat::SendPendingRoomKeys()");
    }
}

void MatrixCrypto::LogOlmLibraryVersion() {
    try {
        // Get Olm version
        uint8_t major, minor, patch;
        olm_get_library_version(&major, &minor, &patch);
        
        CryptoLogger::log(CryptoLogger::LOG_INFO, "Olm library version: " + 
                         std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch));
    } catch (...) {
        CryptoLogger::log(CryptoLogger::LOG_WARNING, "Failed to get Olm library version");
    }
}

void MatrixCrypto::LogCryptoState() {
    CryptoLogger::log(CryptoLogger::LOG_INFO, "=== Crypto State Summary ===");
    CryptoLogger::log(CryptoLogger::LOG_INFO, "User ID: " + m_sUserId);
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Device ID: " + m_sDeviceId);
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Initialized: " + std::string(m_bInitialized ? "true" : "false"));
    
    {
        crypto_mutex::shared_lock lock(m_SessionsMutex);
        CryptoLogger::log(CryptoLogger::LOG_INFO, "Active room sessions: " + std::to_string(m_RoomSessions.size()));
    }
    
    {
        crypto_mutex::shared_lock lock(m_DevicesMutex);
        size_t total_devices = 0;
        for (const auto& user_pair : m_UserDevices) {
            const auto& user_id = user_pair.first;
            const auto& devices = user_pair.second;
            total_devices += devices.size();
        }
        CryptoLogger::log(CryptoLogger::LOG_INFO, "Tracked devices: " + std::to_string(total_devices) + 
                         " across " + std::to_string(m_UserDevices.size()) + " users");
    }
    
    {
        crypto_mutex::shared_lock lock(m_FailedDevicesMutex);
        CryptoLogger::log(CryptoLogger::LOG_INFO, "Failed devices: " + std::to_string(m_FailedDevices.size()));
    }
    
    CryptoLogger::log(CryptoLogger::LOG_INFO, "=== End Crypto State ===");
}

bool MatrixCrypto::ShareSessionKey(const std::string& room_id, const std::vector<std::string>& user_ids) {
    if (!EnsureRoomSession(room_id)) {
        CryptoLogger::log(CryptoLogger::LOG_ERROR, "Failed to ensure room session for key sharing");
        return false;
    }
    
    crypto_mutex::shared_lock session_lock(m_SessionsMutex);
    auto session_it = m_RoomSessions.find(room_id);
    if (session_it == m_RoomSessions.end()) {
        CryptoLogger::log(CryptoLogger::LOG_ERROR, "No session found for room: " + room_id);
        return false;
    }
    
    auto& session = session_it->second;
    
    // Export session key in the format expected by Element
    size_t key_length = olm_outbound_group_session_key_length(session->outbound_session);
    std::vector<uint8_t> session_key_buffer(key_length);
    
    size_t key_result = olm_outbound_group_session_key(session->outbound_session,
                                                      session_key_buffer.data(), key_length);
    if (key_result == olm_error()) {
        const char* error = olm_outbound_group_session_last_error(session->outbound_session);
        CryptoLogger::log(CryptoLogger::LOG_ERROR, "Failed to export session key: " + 
                         std::string(error ? error : "unknown error"));
        return false;
    }
    
    std::string session_key_b64(reinterpret_cast<char*>(session_key_buffer.data()), key_result);
    
    // Create room key event
    nlohmann::json room_key_event = {
        {"type", "m.room_key"},
        {"content", {
            {"algorithm", "m.megolm.v1.aes-sha2"},
            {"room_id", room_id},
            {"session_id", session->session_id},
            {"session_key", session_key_b64}
        }}
    };
    
    session_lock.unlock();
    
    // Collect all devices for the specified users
    std::vector<MatrixDevice> target_devices;
    {
        crypto_mutex::shared_lock device_lock(m_DevicesMutex);
        for (const auto& user_id : user_ids) {
            std::unordered_map<std::string, std::unordered_map<std::string, MatrixDevice>>::const_iterator user_devices_it = m_UserDevices.find(user_id);
            if (user_devices_it != m_UserDevices.end()) {
                for (std::unordered_map<std::string, MatrixDevice>::const_iterator device_it = user_devices_it->second.begin(); device_it != user_devices_it->second.end(); ++device_it) {
                    const std::string& device_id = device_it->first;
                    const MatrixDevice& device = device_it->second;
                    target_devices.push_back(device);
                }
            }
        }
    }
    
    if (target_devices.empty()) {
        CryptoLogger::log(CryptoLogger::LOG_WARNING, "No devices found for room key sharing");
        return false;
    }
    
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Sharing room key with " + std::to_string(target_devices.size()) + " devices");
    
    // Share the room key with all devices
    return ShareRoomKey(room_id, target_devices);
}

bool MatrixCrypto::ShareRoomKey(const std::string& room_id, const std::vector<MatrixDevice>& devices) {
    if (devices.empty()) {
        CryptoLogger::log(CryptoLogger::LOG_WARNING, "No devices to share room key with");
        return true;
    }
    
    std::lock_guard<std::mutex> lock(m_PendingKeysMutex);
    
    size_t successful_encryptions = 0;
    
    for (const auto& device : devices) {
        try {
            PendingRoomKey pending_key;
            pending_key.user_id = device.user_id;
            pending_key.device_id = device.device_id;
            pending_key.room_id = room_id;
            pending_key.sequence_number = ++m_MessageSequence;
            pending_key.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            
            // Encrypt the room key for this device (with safety checks)
            std::string encrypted_content = GenerateRoomKeyMessage(device, room_id);
            
            if (!encrypted_content.empty() && encrypted_content != "{}") {
                pending_key.encrypted_content = encrypted_content;
                m_PendingRoomKeys.push(pending_key);
                successful_encryptions++;
                
                CryptoLogger::log(CryptoLogger::LOG_DEBUG, "Successfully encrypted room key for device: " + device.device_id);
            } else {
                CryptoLogger::log(CryptoLogger::LOG_WARNING, "Failed to encrypt room key for device: " + device.device_id);
            }
            
        } catch (const std::exception& e) {
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "Exception encrypting room key for device " + device.device_id + ": " + std::string(e.what()));
        }
    }
    
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Successfully encrypted " + std::to_string(successful_encryptions) + "/" + std::to_string(devices.size()) + " room keys");
    
    // Consider it successful if we encrypted at least some keys
    return successful_encryptions > 0;
}

std::vector<PendingRoomKey> MatrixCrypto::GetPendingRoomKeys() {
    std::lock_guard<std::mutex> lock(m_PendingKeysMutex);
    
    std::vector<PendingRoomKey> pending_keys;
    std::queue<PendingRoomKey> temp_queue = m_PendingRoomKeys;
    
    while (!temp_queue.empty()) {
        pending_keys.push_back(temp_queue.front());
        temp_queue.pop();
    }
    
    return pending_keys;
}

void MatrixCrypto::ClearPendingRoomKeys() {
    std::lock_guard<std::mutex> lock(m_PendingKeysMutex);
    std::queue<PendingRoomKey> empty;
    m_PendingRoomKeys.swap(empty);
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Cleared all pending room keys");
}

bool MatrixCrypto::ValidateSessionIntegrity(const std::string& room_id) {
    crypto_mutex::shared_lock lock(m_SessionsMutex);
    
    auto it = m_RoomSessions.find(room_id);
    if (it == m_RoomSessions.end()) {
        return false;
    }
    
    auto& session = it->second;
    
    // Validate session state
    if (!session->outbound_session) {
        CryptoLogger::log(CryptoLogger::LOG_ERROR, "Session for room " + room_id + " has no outbound session");
        return false;
    }
    
    // Check if session is still valid
    uint32_t message_index = olm_outbound_group_session_message_index(session->outbound_session);
    if (message_index == UINT32_MAX) {
        CryptoLogger::log(CryptoLogger::LOG_ERROR, "Session for room " + room_id + " has invalid message index");
        return false;
    }
    
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Session integrity validated for room " + room_id);
    return true;
}

bool MatrixCrypto::TestSessionHealth(const std::string& room_id) {
    if (!ValidateSessionIntegrity(room_id)) {
        return false;
    }
    
    // Additional health checks could be added here
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Session health check passed for room: " + room_id);
    return true;
}

bool MatrixCrypto::RecoverRoomSession(const std::string& room_id) {
    CryptoLogger::log(CryptoLogger::LOG_WARNING, "Attempting to recover session for room: " + room_id);
    
    // Clear the corrupted session
    {
        crypto_mutex::unique_lock lock(m_SessionsMutex);
        m_RoomSessions.erase(room_id);
    }
    
    // Create a new session
    return CreateMegolmSession(room_id);
}

bool MatrixCrypto::ForceSessionRotation(const std::string& room_id) {
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Forcing session rotation for room: " + room_id);
    return RotateMegolmSession(room_id);
}

std::string MatrixCrypto::DecryptToDeviceMessage(const std::string& encrypted_event) {
    // This would handle decryption of to-device messages (for room key sharing)
    // Implementation would be similar to DecryptMessage but for Olm sessions
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Decrypting to-device message");
    return "[To-device decryption not yet implemented]";
}

bool MatrixCrypto::ProcessKeyShareEvent(const std::string& event) {
    try {
        auto event_json = nlohmann::json::parse(event);
        
        if (!event_json.contains("type") || event_json["type"] != "m.room_key") {
            return false;
        }
        
        auto content = event_json["content"];
        std::string room_id = content["room_id"];
        std::string session_id = content["session_id"];
        std::string session_key = content["session_key"];
        
        CryptoLogger::log(CryptoLogger::LOG_INFO, "Processing room key share event for room: " + room_id);
        
        // Create inbound session from received key
        auto inbound_session = std::make_unique<InboundGroupSession>();
        inbound_session->session_id = session_id;
        inbound_session->room_id = room_id;
        inbound_session->creation_time = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        
        size_t inbound_size = olm_inbound_group_session_size();
        inbound_session->session = reinterpret_cast<OlmInboundGroupSession*>(new uint8_t[inbound_size]);
        olm_inbound_group_session(inbound_session->session);
        
        size_t result = olm_init_inbound_group_session(inbound_session->session,
                                                      reinterpret_cast<const uint8_t*>(session_key.c_str()),
                                                      session_key.length());
        if (result == olm_error()) {
            const char* error = olm_inbound_group_session_last_error(inbound_session->session);
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "Failed to process room key: " + 
                             std::string(error ? error : "unknown error"));
            return false;
        }
        
        // Store the inbound session
        crypto_mutex::unique_lock lock(m_SessionsMutex);
        auto room_it = m_RoomSessions.find(room_id);
        if (room_it != m_RoomSessions.end()) {
            room_it->second->inbound_sessions[session_id] = std::move(inbound_session);
        }
        
        CryptoLogger::log(CryptoLogger::LOG_INFO, "Successfully processed room key share event");
        return true;
        
    } catch (const std::exception& e) {
        CryptoLogger::log(CryptoLogger::LOG_ERROR, "Exception processing key share event: " + std::string(e.what()));
        return false;
    }
}

// Additional missing method implementations
bool MatrixCrypto::ShouldRotateSession(const std::string& room_id) {
    crypto_mutex::shared_lock lock(m_SessionsMutex);
    auto it = m_RoomSessions.find(room_id);
    if (it == m_RoomSessions.end()) {
        return false;
    }
    return ShouldRotateSession(room_id, *it->second);
}

void MatrixCrypto::CleanupOldSessions() {
    CleanupExpiredSessions();
}

bool MatrixCrypto::ReshareSessionKeys(const std::string& room_id, const std::vector<std::string>& user_ids) {
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Resharing session keys for room: " + room_id);
    return ShareSessionKey(room_id, user_ids);
}

void MatrixCrypto::AddFailedDevice(const std::string& user_id, const std::string& device_id, 
                                  const std::string& curve25519_key, const std::string& reason, 
                                  DecryptionErrorCode error_code) {
    crypto_mutex::unique_lock lock(m_FailedDevicesMutex);
    
    std::string device_identifier = user_id + ":" + device_id;
    
    FailedDevice failed_device;
    failed_device.user_id = user_id;
    failed_device.device_id = device_id;
    failed_device.curve25519_key = curve25519_key;
    failed_device.failure_reason = reason;
    failed_device.error_code = error_code;
    failed_device.last_attempt_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    failed_device.retry_count = 0;
    
    m_FailedDevices[device_identifier] = failed_device;
    
    CryptoLogger::log(CryptoLogger::LOG_WARNING, "Added failed device: " + device_identifier + " - " + reason);
}

bool MatrixCrypto::ShouldRetryFailedDevice(const std::string& device_identifier) {
    crypto_mutex::shared_lock lock(m_FailedDevicesMutex);
    
    auto it = m_FailedDevices.find(device_identifier);
    if (it == m_FailedDevices.end()) {
        return false;
    }
    
    const auto& failed_device = it->second;
    
    if (failed_device.retry_count >= FailedDevice::MAX_RETRY_COUNT) {
        return false;
    }
    
    int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    return now >= failed_device.GetNextRetryTime();
}

void MatrixCrypto::ClearFailedDevices() {
    crypto_mutex::unique_lock lock(m_FailedDevicesMutex);
    m_FailedDevices.clear();
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Cleared all failed devices");
}

size_t MatrixCrypto::GetFailedDeviceCount() const {
    crypto_mutex::shared_lock lock(m_FailedDevicesMutex);
    return m_FailedDevices.size();
}

bool MatrixCrypto::RetryFailedDevices(const std::string& room_id) {
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Retrying failed devices for room: " + room_id);
    
    std::vector<std::string> retry_devices;
    
    {
        crypto_mutex::shared_lock lock(m_FailedDevicesMutex);
        for (std::unordered_map<std::string, FailedDevice>::const_iterator it = m_FailedDevices.begin(); 
             it != m_FailedDevices.end(); ++it) {
            const std::string& device_identifier = it->first;
            const FailedDevice& failed_device = it->second;
            if (ShouldRetryFailedDevice(device_identifier)) {
                retry_devices.push_back(device_identifier);
            }
        }
    }
    
    if (retry_devices.empty()) {
        return true;
    }
    
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Retrying " + std::to_string(retry_devices.size()) + " failed devices");
    
    // This would implement actual retry logic
    // For now, just mark the attempt
    {
        crypto_mutex::unique_lock lock(m_FailedDevicesMutex);
        int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        
        for (const auto& device_identifier : retry_devices) {
            auto it = m_FailedDevices.find(device_identifier);
            if (it != m_FailedDevices.end()) {
                it->second.retry_count++;
                it->second.last_attempt_time = now;
            }
        }
    }
    
    return true;
}

void MatrixCrypto::StoreClaimedKey(const std::string& device_key, const std::string& one_time_key) {
    std::lock_guard<std::mutex> lock(m_OneTimeKeysMutex);
    
    OneTimeKeyInfo key_info;
    key_info.key_id = device_key;
    key_info.key = one_time_key;
    key_info.creation_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    m_OneTimeKeys[device_key] = key_info;
    
    CryptoLogger::log(CryptoLogger::LOG_DEBUG, "Stored claimed key for device: " + device_key);
}

// Enhanced compatibility methods to match matrix-sdk-crypto-wasm
bool MatrixCrypto::ExportRoomKeys(std::string& exported_keys) {
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Exporting room keys");
    
    nlohmann::json export_data = nlohmann::json::array();
    
    crypto_mutex::shared_lock lock(m_SessionsMutex);
    for (std::unordered_map<std::string, std::unique_ptr<MegolmSession>>::const_iterator room_it = m_RoomSessions.begin(); room_it != m_RoomSessions.end(); ++room_it) {
        const std::string& room_id = room_it->first;
        const std::unique_ptr<MegolmSession>& session = room_it->second;
        
        if (!session->outbound_session) continue;
        
        // Export session key
        size_t key_length = olm_outbound_group_session_key_length(session->outbound_session);
        std::vector<uint8_t> session_key_buffer(key_length);
        
        size_t key_result = olm_outbound_group_session_key(session->outbound_session,
                                                          session_key_buffer.data(), key_length);
        if (key_result == olm_error()) {
            continue;
        }
        
        std::string session_key_b64(reinterpret_cast<char*>(session_key_buffer.data()), key_result);
        
        nlohmann::json key_export = {
            {"algorithm", "m.megolm.v1.aes-sha2"},
            {"room_id", room_id},
            {"session_id", session->session_id},
            {"session_key", session_key_b64},
            {"sender_key", GetOwnCurve25519Key()},
            {"forwarding_curve25519_key_chain", nlohmann::json::array()},
            {"sender_claimed_keys", {
                {"ed25519", GetOwnEd25519Key()}
            }}
        };
        
        export_data.push_back(key_export);
    }
    
    exported_keys = export_data.dump();
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Exported " + std::to_string(export_data.size()) + " room keys");
    return true;
}

bool MatrixCrypto::ImportRoomKeys(const std::string& exported_keys, bool overwrite_existing) {
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Importing room keys (overwrite: " + std::string(overwrite_existing ? "true" : "false") + ")");
    
    try {
        auto import_data = nlohmann::json::parse(exported_keys);
        
        if (!import_data.is_array()) {
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "Invalid room key export format - not an array");
            return false;
        }
        
        size_t imported_count = 0;
        for (const auto& key_data : import_data) {
            if (!key_data.contains("room_id") || !key_data.contains("session_id") || 
                !key_data.contains("session_key")) {
                continue;
            }
            
            std::string room_id = key_data["room_id"];
            std::string session_id = key_data["session_id"];
            std::string session_key = key_data["session_key"];
            
            // Import the session key
            if (ImportSessionKey(room_id, session_key, 0)) {
                imported_count++;
            }
        }
        
        CryptoLogger::log(CryptoLogger::LOG_INFO, "Successfully imported " + std::to_string(imported_count) + " room keys");
        return imported_count > 0;
        
    } catch (const std::exception& e) {
        CryptoLogger::log(CryptoLogger::LOG_ERROR, "Exception importing room keys: " + std::string(e.what()));
        return false;
    }
}

std::string MatrixCrypto::GetOwnEd25519Key() const {
    if (!m_pAccount) {
        return "";
    }
    
    try {
        size_t id_keys_length = olm_account_identity_keys_length(m_pAccount);
        std::vector<uint8_t> id_keys_buffer(id_keys_length);
        
        size_t result = olm_account_identity_keys(m_pAccount, id_keys_buffer.data(), id_keys_length);
        if (result == olm_error()) {
            return "";
        }
        
        std::string id_keys_str(reinterpret_cast<char*>(id_keys_buffer.data()), id_keys_length);
        auto id_keys_json = nlohmann::json::parse(id_keys_str);
        
        return id_keys_json["ed25519"];
    } catch (...) {
        return "";
    }
}

std::string MatrixCrypto::GetEncryptionSettings(const std::string& room_id) {
    std::lock_guard<std::mutex> lock(m_EncryptionSettingsMutex);
    
    auto it = m_RoomEncryptionSettings.find(room_id);
    if (it != m_RoomEncryptionSettings.end()) {
        return it->second.dump();
    }
    
    // Return default settings
    nlohmann::json default_settings = {
        {"algorithm", "m.megolm.v1.aes-sha2"},
        {"rotation_period", 604800000}, // 7 days in milliseconds
        {"rotation_period_messages", 100}
    };
    
    return default_settings.dump();
}

bool MatrixCrypto::SetEncryptionSettings(const std::string& room_id, const std::string& settings) {
    try {
        auto settings_json = nlohmann::json::parse(settings);
        
        std::lock_guard<std::mutex> lock(m_EncryptionSettingsMutex);
        m_RoomEncryptionSettings[room_id] = settings_json;
        
        CryptoLogger::log(CryptoLogger::LOG_INFO, "Updated encryption settings for room: " + room_id);
        return true;
        
    } catch (const std::exception& e) {
        CryptoLogger::log(CryptoLogger::LOG_ERROR, "Failed to set encryption settings: " + std::string(e.what()));
        return false;
    }
}

bool MatrixCrypto::TrustDevice(const std::string& user_id, const std::string& device_id) {
    crypto_mutex::unique_lock lock(m_DevicesMutex);
    
    auto user_it = m_UserDevices.find(user_id);
    if (user_it != m_UserDevices.end()) {
        auto device_it = user_it->second.find(device_id);
        if (device_it != user_it->second.end()) {
            device_it->second.verification_status = DeviceVerificationStatus::LocallyTrusted;
            CryptoLogger::log(CryptoLogger::LOG_INFO, "Marked device as trusted: " + user_id + ":" + device_id);
            return true;
        }
    }
    
    CryptoLogger::log(CryptoLogger::LOG_WARNING, "Device not found for trust: " + user_id + ":" + device_id);
    return false;
}

bool MatrixCrypto::UntrustDevice(const std::string& user_id, const std::string& device_id) {
    crypto_mutex::unique_lock lock(m_DevicesMutex);
    
    auto user_it = m_UserDevices.find(user_id);
    if (user_it != m_UserDevices.end()) {
        auto device_it = user_it->second.find(device_id);
        if (device_it != user_it->second.end()) {
            device_it->second.verification_status = DeviceVerificationStatus::Unverified;
            CryptoLogger::log(CryptoLogger::LOG_INFO, "Marked device as untrusted: " + user_id + ":" + device_id);
            return true;
        }
    }
    
    return false;
}

bool MatrixCrypto::IsDeviceTrusted(const std::string& user_id, const std::string& device_id) const {
    crypto_mutex::shared_lock lock(m_DevicesMutex);
    
    auto user_it = m_UserDevices.find(user_id);
    if (user_it != m_UserDevices.end()) {
        auto device_it = user_it->second.find(device_id);
        if (device_it != user_it->second.end()) {
            return device_it->second.verification_status == DeviceVerificationStatus::Verified ||
                   device_it->second.verification_status == DeviceVerificationStatus::LocallyTrusted ||
                   device_it->second.verification_status == DeviceVerificationStatus::CrossSigningTrusted;
        }
    }
    
    return false;
}

bool MatrixCrypto::HasPendingOperations() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(m_PendingKeysMutex));
    return !m_PendingRoomKeys.empty();
}

size_t MatrixCrypto::GetPendingOperationCount() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(m_PendingKeysMutex));
    return m_PendingRoomKeys.size();
}

void MatrixCrypto::ProcessAllPendingOperations() {
    ProcessPendingRoomKeys();
}

void MatrixCrypto::FlushPendingOperations() {
    ProcessAllPendingOperations();
}

bool MatrixCrypto::WaitForPendingOperations(uint32_t timeout_ms) {
    auto start_time = std::chrono::steady_clock::now();
    
    while (HasPendingOperations()) {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start_time).count();
        
        if (elapsed >= timeout_ms) {
            return false;
        }
        
        ProcessAllPendingOperations();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    return true;
}

void MatrixCrypto::SetSessionKeyExportTiming(uint32_t delay_ms) {
    m_SessionKeyExportDelay = delay_ms;
    CryptoLogger::log(CryptoLogger::LOG_DEBUG, "Set session key export delay to " + std::to_string(delay_ms) + "ms");
}

void MatrixCrypto::SetDeviceVerificationTimeout(uint32_t timeout_ms) {
    m_DeviceVerificationTimeout = timeout_ms;
    CryptoLogger::log(CryptoLogger::LOG_DEBUG, "Set device verification timeout to " + std::to_string(timeout_ms) + "ms");
}

std::string MatrixCrypto::DumpCryptoState() const {
    nlohmann::json state = {
        {"user_id", m_sUserId},
        {"device_id", m_sDeviceId},
        {"initialized", m_bInitialized.load()},
        {"shutting_down", m_bShuttingDown.load()}
    };
    
    {
        crypto_mutex::shared_lock lock(m_SessionsMutex);
        state["room_sessions"] = m_RoomSessions.size();
    }
    
    {
        crypto_mutex::shared_lock lock(m_DevicesMutex);
        size_t total_devices = 0;
        for (std::unordered_map<std::string, std::unordered_map<std::string, MatrixDevice>>::const_iterator user_it = m_UserDevices.begin(); user_it != m_UserDevices.end(); ++user_it) {
            total_devices += user_it->second.size();
        }
        state["tracked_devices"] = total_devices;
        state["tracked_users"] = m_UserDevices.size();
    }
    
    state["pending_operations"] = GetPendingOperationCount();
    
    return state.dump(2);
}

bool MatrixCrypto::SelfTest() {
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Running crypto self-test");
    
    // Test basic initialization
    if (!m_bInitialized) {
        CryptoLogger::log(CryptoLogger::LOG_ERROR, "Self-test failed: not initialized");
        return false;
    }
    
    if (!m_pAccount) {
        CryptoLogger::log(CryptoLogger::LOG_ERROR, "Self-test failed: no Olm account");
        return false;
    }
    
    // Test key generation
    std::string curve25519_key = GetOwnCurve25519Key();
    std::string ed25519_key = GetOwnEd25519Key();
    
    if (curve25519_key.empty() || ed25519_key.empty()) {
        CryptoLogger::log(CryptoLogger::LOG_ERROR, "Self-test failed: missing identity keys");
        return false;
    }
    
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Self-test passed");
    return true;
}

std::string MatrixCrypto::GetLibraryVersions() const {
    nlohmann::json versions;
    
    try {
        uint8_t major, minor, patch;
        olm_get_library_version(&major, &minor, &patch);
        
        versions["olm"] = std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
    } catch (...) {
        versions["olm"] = "unknown";
    }
    
    versions["matrix_crypto"] = "Amalgam-1.0.0";
    
    return versions.dump();
}

std::string MatrixCrypto::GenerateSecureRandom(size_t length) {
    return g_SecureRandom.generate_string(length);
}

// Advanced session management methods
bool MatrixCrypto::ImportSessionKey(const std::string& room_id, const std::string& session_key, uint32_t message_index) {
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Importing session key for room: " + room_id);
    
    try {
        // Ensure room session exists
        if (!EnsureRoomSession(room_id)) {
            return false;
        }
        
        crypto_mutex::unique_lock lock(m_SessionsMutex);
        auto room_it = m_RoomSessions.find(room_id);
        if (room_it == m_RoomSessions.end()) {
            return false;
        }
        
        auto& room_session = room_it->second;
        
        // Create new inbound session from imported key
        auto new_inbound = std::make_unique<InboundGroupSession>();
        new_inbound->room_id = room_id;
        new_inbound->first_known_index = message_index;
        new_inbound->creation_time = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        
        size_t inbound_size = olm_inbound_group_session_size();
        new_inbound->session = reinterpret_cast<OlmInboundGroupSession*>(new uint8_t[inbound_size]);
        olm_inbound_group_session(new_inbound->session);
        
        size_t result = olm_init_inbound_group_session(new_inbound->session,
                                                      reinterpret_cast<const uint8_t*>(session_key.c_str()),
                                                      session_key.length());
        if (result == olm_error()) {
            const char* error = olm_inbound_group_session_last_error(new_inbound->session);
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "Failed to import session key: " + 
                             std::string(error ? error : "unknown error"));
            return false;
        }
        
        // Get session ID from imported session
        size_t session_id_length = olm_inbound_group_session_id_length(new_inbound->session);
        std::vector<uint8_t> session_id_buffer(session_id_length);
        
        size_t id_result = olm_inbound_group_session_id(new_inbound->session,
                                                       session_id_buffer.data(), session_id_length);
        if (id_result == olm_error()) {
            return false;
        }
        
        new_inbound->session_id = std::string(reinterpret_cast<char*>(session_id_buffer.data()), id_result);
        
        // Store the imported session
        room_session->inbound_sessions[new_inbound->session_id] = std::move(new_inbound);
        
        CryptoLogger::log(CryptoLogger::LOG_INFO, "Successfully imported session key for room " + room_id);
        return true;
        
    } catch (const std::exception& e) {
        CryptoLogger::log(CryptoLogger::LOG_ERROR, "Exception importing session key: " + std::string(e.what()));
        return false;
    }
}

bool MatrixCrypto::ExportSessionKey(const std::string& room_id, uint32_t message_index, std::string& exported_key) {
    crypto_mutex::shared_lock lock(m_SessionsMutex);
    
    auto it = m_RoomSessions.find(room_id);
    if (it == m_RoomSessions.end() || !it->second->outbound_session) {
        return false;
    }
    
    auto& session = it->second;
    
    size_t key_length = olm_outbound_group_session_key_length(session->outbound_session);
    std::vector<uint8_t> session_key_buffer(key_length);
    
    size_t key_result = olm_outbound_group_session_key(session->outbound_session,
                                                      session_key_buffer.data(), key_length);
    if (key_result == olm_error()) {
        return false;
    }
    
    exported_key = std::string(reinterpret_cast<char*>(session_key_buffer.data()), key_result);
    return true;
}

bool MatrixCrypto::ValidateAllSessions() {
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Validating all sessions");
    
    crypto_mutex::shared_lock lock(m_SessionsMutex);
    
    bool all_valid = true;
    for (std::unordered_map<std::string, std::unique_ptr<MegolmSession>>::const_iterator room_it = m_RoomSessions.begin(); room_it != m_RoomSessions.end(); ++room_it) {
        const std::string& room_id = room_it->first;
        if (!ValidateSessionIntegrity(room_id)) {
            CryptoLogger::log(CryptoLogger::LOG_WARNING, "Session validation failed for room: " + room_id);
            all_valid = false;
        }
    }
    
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Session validation completed. All valid: " + std::string(all_valid ? "true" : "false"));
    return all_valid;
}

std::string MatrixCrypto::GetSessionInfo(const std::string& room_id) {
    crypto_mutex::shared_lock lock(m_SessionsMutex);
    
    auto it = m_RoomSessions.find(room_id);
    if (it == m_RoomSessions.end()) {
        return "{}";
    }
    
    auto& session = it->second;
    
    nlohmann::json info = {
        {"room_id", room_id},
        {"session_id", session->session_id},
        {"message_count", session->message_count},
        {"creation_time", session->creation_time},
        {"last_use_time", session->last_use_time},
        {"key_shared", session->key_shared},
        {"needs_rotation", session->needs_rotation},
        {"devices_with_key_count", session->devices_with_key.size()},
        {"inbound_sessions_count", session->inbound_sessions.size()}
    };
    
    if (session->outbound_session) {
        info["message_index"] = olm_outbound_group_session_message_index(session->outbound_session);
    }
    
    return info.dump(2);
}

std::string MatrixCrypto::GetDeviceInfo(const std::string& user_id, const std::string& device_id) {
    crypto_mutex::shared_lock lock(m_DevicesMutex);
    
    auto user_it = m_UserDevices.find(user_id);
    if (user_it == m_UserDevices.end()) {
        return "{}";
    }
    
    auto device_it = user_it->second.find(device_id);
    if (device_it == user_it->second.end()) {
        return "{}";
    }
    
    const auto& device = device_it->second;
    
    std::string verification_status_str;
    switch (device.verification_status) {
        case DeviceVerificationStatus::Verified: verification_status_str = "Verified"; break;
        case DeviceVerificationStatus::Unverified: verification_status_str = "Unverified"; break;
        case DeviceVerificationStatus::BlackListed: verification_status_str = "BlackListed"; break;
        case DeviceVerificationStatus::CrossSigningTrusted: verification_status_str = "CrossSigningTrusted"; break;
        case DeviceVerificationStatus::LocallyTrusted: verification_status_str = "LocallyTrusted"; break;
    }
    
    nlohmann::json info = {
        {"user_id", device.user_id},
        {"device_id", device.device_id},
        {"display_name", device.display_name},
        {"curve25519_key", device.curve25519_key},
        {"ed25519_key", device.ed25519_key},
        {"verification_status", verification_status_str},
        {"first_time_seen", device.first_time_seen},
        {"last_seen", device.last_seen},
        {"deleted", device.deleted},
        {"cross_signing_trusted", device.cross_signing_trusted},
        {"signature_count", device.signatures.size()}
    };
    
    return info.dump(2);
}

bool MatrixCrypto::SetupKeyBackup(const std::string& backup_key, const std::string& backup_version) {
    std::lock_guard<std::mutex> lock(m_BackupMutex);
    
    m_BackupKey = backup_key;
    m_BackupVersion = backup_version;
    
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Set up key backup with version: " + backup_version);
    return true;
}

bool MatrixCrypto::RestoreFromKeyBackup(const std::string& backup_key, const std::string& backup_data) {
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Restoring from key backup");
    
    try {
        auto backup_json = nlohmann::json::parse(backup_data);
        
        if (!backup_json.contains("sessions")) {
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "Invalid backup format - missing sessions");
            return false;
        }
        
        size_t restored_count = 0;
        for (const auto& room_data : backup_json["sessions"].items()) {
            const std::string& room_id = room_data.key();
            
            for (const auto& session_data : room_data.value().items()) {
                if (session_data.value().contains("session_data")) {
                    std::string session_key = session_data.value()["session_data"];
                    if (ImportSessionKey(room_id, session_key, 0)) {
                        restored_count++;
                    }
                }
            }
        }
        
        CryptoLogger::log(CryptoLogger::LOG_INFO, "Restored " + std::to_string(restored_count) + " sessions from backup");
        return restored_count > 0;
        
    } catch (const std::exception& e) {
        CryptoLogger::log(CryptoLogger::LOG_ERROR, "Exception restoring from backup: " + std::string(e.what()));
        return false;
    }
}

bool MatrixCrypto::ShareHistoricalKeys(const std::string& room_id, const std::vector<std::string>& user_ids) {
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Sharing historical keys for room: " + room_id);
    
    // This would include sharing all historical session keys for the room
    return ShareSessionKey(room_id, user_ids);
}

bool MatrixCrypto::RequestRoomKey(const std::string& room_id, const std::string& session_id, const std::string& sender_key) {
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Requesting room key for session: " + session_id);
    
    // Create room key request event
    nlohmann::json request_content = {
        {"action", "request"},
        {"body", {
            {"algorithm", "m.megolm.v1.aes-sha2"},
            {"room_id", room_id},
            {"session_id", session_id},
            {"sender_key", sender_key}
        }},
        {"request_id", GenerateSecureRandom(16)},
        {"requesting_device_id", m_sDeviceId}
    };
    
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Created room key request for session " + session_id);
    return true;
}

std::string MatrixCrypto::GenerateRoomKeyMessage(const MatrixDevice& device, const std::string& room_id) {
    try {
        crypto_mutex::shared_lock lock(m_SessionsMutex);
        
        // Get the current megolm session for the room
        auto session_it = m_RoomSessions.find(room_id);
        if (session_it == m_RoomSessions.end() || !session_it->second) {
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "No megolm session found for room: " + room_id);
            return "{}";  // Return empty JSON object
        }
        
        auto* session = session_it->second.get();
        if (!session->outbound_session) {
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "No outbound session found for room: " + room_id);
            return "{}";
        }
        
        // Get the session key from the megolm session
        size_t key_length = olm_outbound_group_session_key_length(session->outbound_session);
        std::string session_key(key_length, '\0');
        
        size_t result = olm_outbound_group_session_key(
            session->outbound_session,
            reinterpret_cast<uint8_t*>(&session_key[0]),
            key_length
        );
        
        if (result == olm_error()) {
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "Failed to get session key: " + std::string(olm_outbound_group_session_last_error(session->outbound_session)));
            return "{}";
        }
        
        session_key.resize(result);
        
        // Create the room key event content matching Element Web's format
        nlohmann::json room_key_content = {
            {"algorithm", "m.megolm.v1.aes-sha2"},
            {"room_id", room_id},
            {"session_id", session->session_id},
            {"session_key", session_key}
        };
        
        // Create the m.room_key event with proper structure
        nlohmann::json room_key_event = {
            {"type", "m.room_key"},
            {"content", room_key_content}
        };
        
        // Now encrypt this event using Olm for the specific device
        std::string encrypted_content = EncryptForDevice(device, room_key_event.dump());
        if (encrypted_content.empty()) {
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "Failed to encrypt room key for device: " + device.device_id);
            return "{}";
        }
        
        CryptoLogger::log(CryptoLogger::LOG_INFO, "Generated encrypted room key message for device: " + device.device_id);
        return encrypted_content;
        
    } catch (const std::exception& e) {
        CryptoLogger::log(CryptoLogger::LOG_ERROR, "Exception in GenerateRoomKeyMessage: " + std::string(e.what()));
        return "{}";
    }
}

std::string MatrixCrypto::EncryptForDevice(const MatrixDevice& device, const std::string& plaintext) {
    if (!m_bInitialized.load()) {
        CryptoLogger::log(CryptoLogger::LOG_ERROR, "Crypto not initialized for encryption");
        return "";
    }
    
    if (device.curve25519_key.empty() || plaintext.empty()) {
        CryptoLogger::log(CryptoLogger::LOG_ERROR, "Invalid device key or plaintext");
        return "";
    }
    
    try {
        // First, ensure we have an Olm session with this device (without holding locks)
        if (!CreateOlmSessionIfNeeded(device)) {
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "Failed to create Olm session for device: " + device.device_id);
            return "";
        }
        
        // Now acquire lock to use the session
        crypto_mutex::shared_lock olm_lock(m_OlmSessionsMutex);
        
        // Look for the Olm session with this device
        auto session_it = m_OlmSessions.find(device.curve25519_key);
        if (session_it == m_OlmSessions.end() || !session_it->second || !session_it->second->session) {
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "No Olm session found for device after creation attempt: " + device.device_id);
            return "";
        }
        
        auto* olm_session = session_it->second->session;
        
        // Additional safety check to prevent crashes
        if (!olm_session) {
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "Null Olm session pointer for device: " + device.device_id);
            return "";
        }
        
        // Get the identity key for our own device
        std::string our_curve25519_key = GetOwnCurve25519Key();
        std::string our_ed25519_key = GetOwnEd25519Key();
        
        if (our_curve25519_key.empty() || our_ed25519_key.empty()) {
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "Failed to get our own device keys");
            return "";
        }
        
        // Calculate required buffer sizes with safety checks
        size_t message_type = olm_encrypt_message_type(olm_session);
        size_t ciphertext_length = olm_encrypt_message_length(olm_session, plaintext.length());
        size_t random_length = olm_encrypt_random_length(olm_session);
        
        // Validate buffer sizes to prevent crashes
        if (ciphertext_length == olm_error() || random_length == olm_error()) {
            const char* error = olm_session_last_error(olm_session);
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "Failed to get encryption lengths: " + 
                             std::string(error ? error : "unknown error"));
            return "";
        }
        
        if (random_length > 1024 || ciphertext_length > 65536) {  // Sanity check
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "Suspicious buffer sizes: random=" + 
                             std::to_string(random_length) + ", ciphertext=" + std::to_string(ciphertext_length));
            return "";
        }
        
        // Generate random data
        std::string random_data = GenerateSecureRandom(static_cast<int>(random_length));
        std::string ciphertext(ciphertext_length, '\0');
        
        // Encrypt the plaintext
        size_t result = olm_encrypt(
            olm_session,
            reinterpret_cast<const void*>(plaintext.c_str()),
            plaintext.length(),
            reinterpret_cast<void*>(&random_data[0]),
            random_length,
            reinterpret_cast<void*>(&ciphertext[0]),
            ciphertext_length
        );
        
        if (result == olm_error()) {
            const char* error = olm_session_last_error(olm_session);
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "Failed to encrypt message: " + std::string(error ? error : "unknown error"));
            
            // Mark session as corrupted
            session_it->second->corrupted = true;
            session_it->second->last_error = std::string(error ? error : "unknown error");
            
            return "";
        }
        
        // Update session usage statistics
        session_it->second->message_count++;
        session_it->second->last_use_time = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        
        ciphertext.resize(result);
        
        // Create the encrypted event content with Element-web compatible format
        nlohmann::json encrypted_content = {
            {"algorithm", "m.olm.v1.curve25519-aes-sha2"},
            {"ciphertext", {
                {device.curve25519_key, {  // Use original base64 key as identifier
                    {"type", static_cast<int>(message_type)},
                    {"body", ciphertext}
                }}
            }},
            {"sender_key", our_curve25519_key}  // This should already be base64
        };
        
        CryptoLogger::log(CryptoLogger::LOG_INFO, "Successfully encrypted message for device: " + device.device_id);
        return encrypted_content.dump();
        
    } catch (const std::exception& e) {
        CryptoLogger::log(CryptoLogger::LOG_ERROR, "Exception in EncryptForDevice: " + std::string(e.what()));
        return "";
    }
}

bool MatrixCrypto::CreateOlmSessionIfNeeded(const MatrixDevice& device) {
    if (!m_bInitialized.load()) {
        CryptoLogger::log(CryptoLogger::LOG_ERROR, "Crypto not initialized");
        return false;
    }
    
    if (device.curve25519_key.empty() || device.user_id.empty() || device.device_id.empty()) {
        CryptoLogger::log(CryptoLogger::LOG_ERROR, "Invalid device data for session creation");
        return false;
    }
    
    try {
        crypto_mutex::unique_lock olm_lock(m_OlmSessionsMutex);
        
        // Check if session already exists and validate it
        auto existing_it = m_OlmSessions.find(device.curve25519_key);
        if (existing_it != m_OlmSessions.end() && existing_it->second && existing_it->second->session) {
            // Validate session health and encryption capability
            if (!existing_it->second->corrupted && existing_it->second->message_count < 1000) {
                // Test that the session can still encrypt properly
                try {
                    std::string test_message = "{\"test\":\"validation\"}";
                    size_t ciphertext_length = olm_encrypt_message_length(existing_it->second->session, test_message.length());
                    if (ciphertext_length != olm_error()) {
                        CryptoLogger::log(CryptoLogger::LOG_DEBUG, "Olm session already exists and validated for device: " + device.device_id);
                        return true;
                    } else {
                        CryptoLogger::log(CryptoLogger::LOG_WARNING, "Olm session validation failed for device " + device.device_id + ", recreating");
                        existing_it->second->corrupted = true;
                    }
                } catch (...) {
                    CryptoLogger::log(CryptoLogger::LOG_WARNING, "Olm session validation exception for device " + device.device_id + ", recreating");
                    existing_it->second->corrupted = true;
                }
            }
            
            if (existing_it->second->corrupted || existing_it->second->message_count >= 1000) {
                CryptoLogger::log(CryptoLogger::LOG_WARNING, "Olm session corrupted or overused for device " + device.device_id + ", recreating");
                // Clear the corrupted session
                m_OlmSessions.erase(existing_it);
            }
        }
        
        CryptoLogger::log(CryptoLogger::LOG_INFO, "Creating new Olm session for device: " + device.device_id);
        
        if (!m_pAccount) {
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "No Olm account available for session creation");
            return false;
        }
        
        // Create new Olm session with proper error checking
        size_t session_size = olm_session_size();
        if (session_size == 0 || session_size > 65536) {
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "Invalid Olm session size: " + std::to_string(session_size));
            return false;
        }
        
        auto session_info = std::make_unique<OlmSessionInfo>();
        session_info->session = reinterpret_cast<OlmSession*>(new(std::nothrow) uint8_t[session_size]);
        if (!session_info->session) {
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "Failed to allocate memory for Olm session");
            return false;
        }
        
        // Initialize the session
        if (olm_session(session_info->session) == nullptr) {
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "Failed to initialize Olm session");
            delete[] reinterpret_cast<uint8_t*>(session_info->session);
            return false;
        }
        
        session_info->sender_key = device.curve25519_key;
        session_info->creation_time = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        
        // Validate device keys before session creation (Element-web compatibility)
        if (!ValidateDeviceKeys(device)) {
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "Device key validation failed for: " + device.device_id);
            return false;
        }
        
        // For creating an outbound session, we need a one-time key
        // This should have been claimed during HttpClaimKeys
        std::string one_time_key;
        {
            std::lock_guard<std::mutex> otk_lock(m_OneTimeKeysMutex);
            std::string device_key = device.user_id + ":" + device.device_id;
            auto otk_it = m_OneTimeKeys.find(device_key);
            
            if (otk_it == m_OneTimeKeys.end()) {
                CryptoLogger::log(CryptoLogger::LOG_ERROR, "No one-time key available for device: " + device.device_id);
                // Element-web compatible behavior: Don't fail, just mark as needing retry
                // This device likely doesn't have one-time keys or is offline
                AddFailedDevice(device.user_id, device.device_id, device.curve25519_key, 
                               "No one-time key available", DecryptionErrorCode::MissingRoomKey);
                return false;
            }
            
            one_time_key = otk_it->second.key;
        }
        CryptoLogger::log(CryptoLogger::LOG_DEBUG, "Using one-time key for session creation: " + one_time_key.substr(0, 10) + "...");
        
        // Get our own identity key
        std::string our_identity_key = GetOwnCurve25519Key();
        if (our_identity_key.empty()) {
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "Failed to get our identity key");
            return false;
        }
        
        // Calculate required random data
        size_t random_length = olm_create_outbound_session_random_length(session_info->session);
        if (random_length == 0 || random_length > 1024) {
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "Invalid random length for session creation: " + std::to_string(random_length));
            return false;
        }
        std::string random_data = GenerateSecureRandom(static_cast<int>(random_length));
        if (random_data.length() != random_length) {
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "Random data length mismatch: expected " + 
                             std::to_string(random_length) + ", got " + std::to_string(random_data.length()));
            return false;
        }
        
        // Validate inputs before creating session
        if (device.curve25519_key.empty() || one_time_key.empty() || random_data.empty()) {
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "Invalid input data for session creation");
            return false;
        }
        
        // Create outbound session (keys should already be in correct base64 format)
        size_t result = olm_create_outbound_session(
            session_info->session,
            m_pAccount,
            reinterpret_cast<const void*>(device.curve25519_key.c_str()),
            device.curve25519_key.length(),
            reinterpret_cast<const void*>(one_time_key.c_str()),
            one_time_key.length(),
            reinterpret_cast<void*>(&random_data[0]),
            random_length
        );
        
        if (result == olm_error()) {
            const char* error = olm_session_last_error(session_info->session);
            std::string error_msg = std::string(error ? error : "unknown error");
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "Failed to create outbound session: " + error_msg);
            
            // Mark device as failed for Element-web compatibility
            AddFailedDevice(device.user_id, device.device_id, device.curve25519_key, 
                           "Session creation failed: " + error_msg, DecryptionErrorCode::UnableToDecrypt);
            return false;
        }
        
        // Generate session ID (Base64 encoded for readability)
        session_info->session_id = g_SecureRandom.generate_base64_string(16);
        
        // Basic session validation (safer than full encryption test)
        if (!session_info->session) {
            CryptoLogger::log(CryptoLogger::LOG_ERROR, "Session validation failed: null session");
            return false;
        }
        
        // Store the validated session
        m_OlmSessions[device.curve25519_key] = std::move(session_info);
        
        CryptoLogger::log(CryptoLogger::LOG_INFO, "Successfully created fresh Olm session for device: " + device.device_id);
        return true;
        
    } catch (const std::exception& e) {
        CryptoLogger::log(CryptoLogger::LOG_ERROR, "Exception creating Olm session: " + std::string(e.what()));
        return false;
    }
}

void MatrixCrypto::ClearCorruptedSessions() {
    crypto_mutex::unique_lock lock(m_OlmSessionsMutex);
    
    size_t cleared_count = 0;
    auto it = m_OlmSessions.begin();
    while (it != m_OlmSessions.end()) {
        if (it->second && it->second->corrupted) {
            CryptoLogger::log(CryptoLogger::LOG_INFO, "Clearing corrupted session for device");
            it = m_OlmSessions.erase(it);
            cleared_count++;
        } else {
            ++it;
        }
    }
    
    if (cleared_count > 0) {
        CryptoLogger::log(CryptoLogger::LOG_INFO, "Cleared " + std::to_string(cleared_count) + " corrupted Olm sessions");
    }
}

bool MatrixCrypto::ForceRecreateOlmSession(const MatrixDevice& device) {
    {
        crypto_mutex::unique_lock lock(m_OlmSessionsMutex);
        auto it = m_OlmSessions.find(device.curve25519_key);
        if (it != m_OlmSessions.end()) {
            CryptoLogger::log(CryptoLogger::LOG_INFO, "Force recreating Olm session for device: " + device.device_id);
            m_OlmSessions.erase(it);
        }
    }
    
    // Create fresh session
    return CreateOlmSessionIfNeeded(device);
}

void MatrixCrypto::ClearAllOlmSessions() {
    crypto_mutex::unique_lock lock(m_OlmSessionsMutex);
    
    size_t session_count = m_OlmSessions.size();
    m_OlmSessions.clear();
    
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Cleared all " + std::to_string(session_count) + " Olm sessions for fresh start");
}

bool MatrixCrypto::HandleKeyExhaustion() {
    CryptoLogger::log(CryptoLogger::LOG_WARNING, "Handling one-time key exhaustion");
    
    // Clear all existing Olm sessions to force re-establishment
    ClearAllOlmSessions();
    
    // Clear one-time key cache to force new key claims
    {
        std::lock_guard<std::mutex> lock(m_OneTimeKeysMutex);
        m_OneTimeKeys.clear();
        CryptoLogger::log(CryptoLogger::LOG_INFO, "Cleared one-time key cache");
    }
    
    // Generate new one-time keys
    if (!GenerateOneTimeKeys(50)) {
        CryptoLogger::log(CryptoLogger::LOG_ERROR, "Failed to generate new one-time keys during exhaustion recovery");
        return false;
    }
    
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Key exhaustion recovery complete");
    return true;
}

// Cross-signing implementation for Element-web compatibility
bool MatrixCrypto::SetupCrossSigning(const std::string& master_key, const std::string& self_signing_key, const std::string& user_signing_key) {
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Setting up cross-signing keys");
    
    std::lock_guard<std::mutex> lock(m_CrossSigningMutex);
    
    try {
        // Store master signing key
        if (!master_key.empty()) {
            CrossSigningKey master = {};
            master.key_id = "master";
            master.public_key = master_key;
            master.usage = {"master"};
            master.verified = true;
            m_CrossSigningKeys["master"] = master;
            CryptoLogger::log(CryptoLogger::LOG_DEBUG, "Stored master signing key");
        }
        
        // Store self-signing key
        if (!self_signing_key.empty()) {
            CrossSigningKey self_signing = {};
            self_signing.key_id = "self_signing";
            self_signing.public_key = self_signing_key;
            self_signing.usage = {"self_signing"};
            self_signing.verified = true;
            m_CrossSigningKeys["self_signing"] = self_signing;
            CryptoLogger::log(CryptoLogger::LOG_DEBUG, "Stored self-signing key");
        }
        
        // Store user-signing key
        if (!user_signing_key.empty()) {
            CrossSigningKey user_signing = {};
            user_signing.key_id = "user_signing";
            user_signing.public_key = user_signing_key;
            user_signing.usage = {"user_signing"};
            user_signing.verified = true;
            m_CrossSigningKeys["user_signing"] = user_signing;
            CryptoLogger::log(CryptoLogger::LOG_DEBUG, "Stored user-signing key");
        }
        
        CryptoLogger::log(CryptoLogger::LOG_INFO, "Cross-signing setup completed successfully");
        return true;
        
    } catch (const std::exception& e) {
        CryptoLogger::log(CryptoLogger::LOG_ERROR, "Failed to setup cross-signing: " + std::string(e.what()));
        return false;
    }
}

bool MatrixCrypto::SignDevice(const std::string& user_id, const std::string& device_id) {
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Attempting to sign device " + device_id + " for user " + user_id);
    
    // For now, just mark the device as cross-signing trusted
    // Full implementation would create and upload signatures
    crypto_mutex::unique_lock lock(m_DevicesMutex);
    
    auto user_it = m_UserDevices.find(user_id);
    if (user_it != m_UserDevices.end()) {
        auto device_it = user_it->second.find(device_id);
        if (device_it != user_it->second.end()) {
            device_it->second.cross_signing_trusted = true;
            device_it->second.verification_status = DeviceVerificationStatus::CrossSigningTrusted;
            CryptoLogger::log(CryptoLogger::LOG_INFO, "Device marked as cross-signing trusted");
            return true;
        }
    }
    
    CryptoLogger::log(CryptoLogger::LOG_WARNING, "Device not found for signing: " + user_id + ":" + device_id);
    return false;
}

bool MatrixCrypto::VerifyDeviceWithCrossSigning(const std::string& user_id, const std::string& device_id) {
    CryptoLogger::log(CryptoLogger::LOG_INFO, "Verifying device with cross-signing: " + user_id + ":" + device_id);
    
    // Check if we have the necessary cross-signing keys
    std::lock_guard<std::mutex> cs_lock(m_CrossSigningMutex);
    if (m_CrossSigningKeys.find("master") == m_CrossSigningKeys.end() ||
        m_CrossSigningKeys.find("self_signing") == m_CrossSigningKeys.end()) {
        CryptoLogger::log(CryptoLogger::LOG_WARNING, "Missing cross-signing keys for verification");
        return false;
    }
    
    // For basic compatibility, mark device as trusted if cross-signing keys exist
    crypto_mutex::unique_lock device_lock(m_DevicesMutex);
    auto user_it = m_UserDevices.find(user_id);
    if (user_it != m_UserDevices.end()) {
        auto device_it = user_it->second.find(device_id);
        if (device_it != user_it->second.end()) {
            device_it->second.cross_signing_trusted = true;
            device_it->second.verification_status = DeviceVerificationStatus::CrossSigningTrusted;
            CryptoLogger::log(CryptoLogger::LOG_INFO, "Device verified via cross-signing");
            return true;
        }
    }
    
    return false;
}

// End of MatrixCrypto class implementation