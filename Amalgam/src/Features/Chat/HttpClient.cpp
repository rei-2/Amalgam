// This file is completely isolated from the main codebase to avoid macro conflicts
// Only include system headers first
#include <string>
#include <map>
#include <cstdio>

// Undefine problematic Windows macros before including CPR
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#ifdef Left
#undef Left
#endif
#ifdef Right
#undef Right
#endif
#ifdef PostMessage
#undef PostMessage
#endif

// Include CPR in isolation
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

// Include our header after CPR
#include "HttpClient.h"

// Static callback for logging
static void(*g_LogCallback)(const std::string&, const std::string&) = nullptr;

// Debug logging function
void LogMatrixDebug(const std::string& message) {
#ifdef _DEBUG
    if (g_LogCallback) {
        g_LogCallback("[MATRIX-HTTP]", message);
    }
#endif
}

namespace HttpClient 
{
    HttpResponse Get(const std::string& url, const std::map<std::string, std::string>& headers)
    {
        try 
        {
            // Log request details
            LogMatrixDebug("[HTTP] GET Request: " + url);
            LogMatrixDebug("[HTTP] Headers:");
            for (const auto& header : headers) 
            {
                // Don't log sensitive headers in full
                if (header.first == "Authorization") {
                    LogMatrixDebug("[HTTP]   " + header.first + ": Bearer [REDACTED]");
                } else {
                    LogMatrixDebug("[HTTP]   " + header.first + ": " + header.second);
                }
            }
            
            cpr::Header cprHeaders;
            for (const auto& header : headers) 
            {
                cprHeaders[header.first] = header.second;
            }

            // Add common browser User-Agent if not already set
            if (cprHeaders.find("User-Agent") == cprHeaders.end()) {
                cprHeaders["User-Agent"] = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/131.0.0.0 Safari/537.36";
            }
            
            auto response = cpr::Get(cpr::Url{url}, cprHeaders, cpr::VerifySsl{false});
            
            // Log response details
            LogMatrixDebug("[HTTP] GET Response: Status " + std::to_string(response.status_code));
            LogMatrixDebug("[HTTP] Response Body: " + response.text);
            LogMatrixDebug("[HTTP] ===================================");
            
            return {
                static_cast<int>(response.status_code),
                response.text,
                response.status_code >= 200 && response.status_code < 300
            };
        }
        catch (const std::exception& e)
        {
            LogMatrixDebug("[HTTP] GET Exception: " + std::string(e.what()));
            return { 0, "HTTP Error: " + std::string(e.what()), false };
        }
    }

    HttpResponse Post(const std::string& url, const std::string& body, const std::map<std::string, std::string>& headers)
    {
        try 
        {
            // Log request details
            LogMatrixDebug("[HTTP] POST Request: " + url);
            LogMatrixDebug("[HTTP] Headers:");
            for (const auto& header : headers) 
            {
                // Don't log sensitive headers in full
                if (header.first == "Authorization") {
                    LogMatrixDebug("[HTTP]   " + header.first + ": Bearer [REDACTED]");
                } else {
                    LogMatrixDebug("[HTTP]   " + header.first + ": " + header.second);
                }
            }
            LogMatrixDebug("[HTTP] Request Body: " + body);
            
            cpr::Header cprHeaders;
            for (const auto& header : headers) 
            {
                cprHeaders[header.first] = header.second;
            }

            // Add common browser User-Agent if not already set
            if (cprHeaders.find("User-Agent") == cprHeaders.end()) {
                cprHeaders["User-Agent"] = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/131.0.0.0 Safari/537.36";
            }
            
            auto response = cpr::Post(cpr::Url{url}, cpr::Body{body}, cprHeaders, cpr::VerifySsl{false});
            
            // Log response details
            LogMatrixDebug("[HTTP] POST Response: Status " + std::to_string(response.status_code));
            LogMatrixDebug("[HTTP] Response Body: " + response.text);
            LogMatrixDebug("[HTTP] ===================================");
            
            return {
                static_cast<int>(response.status_code),
                response.text,
                response.status_code >= 200 && response.status_code < 300
            };
        }
        catch (const std::exception& e)
        {
            LogMatrixDebug("[HTTP] POST Exception: " + std::string(e.what()));
            return { 0, "HTTP Error: " + std::string(e.what()), false };
        }
    }

    HttpResponse Put(const std::string& url, const std::string& body, const std::map<std::string, std::string>& headers)
    {
        try 
        {
            // Log request details
            LogMatrixDebug("[HTTP] PUT Request: " + url);
            LogMatrixDebug("[HTTP] Headers:");
            for (const auto& header : headers) 
            {
                // Don't log sensitive headers in full
                if (header.first == "Authorization") {
                    LogMatrixDebug("[HTTP]   " + header.first + ": Bearer [REDACTED]");
                } else {
                    LogMatrixDebug("[HTTP]   " + header.first + ": " + header.second);
                }
            }
            LogMatrixDebug("[HTTP] Request Body: " + body);
            
            cpr::Header cprHeaders;
            for (const auto& header : headers) 
            {
                cprHeaders[header.first] = header.second;
            }

            // Add common browser User-Agent if not already set
            if (cprHeaders.find("User-Agent") == cprHeaders.end()) {
                cprHeaders["User-Agent"] = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/131.0.0.0 Safari/537.36";
            }
            
            auto response = cpr::Put(cpr::Url{url}, cpr::Body{body}, cprHeaders, cpr::VerifySsl{false});
            
            // Log response details
            LogMatrixDebug("[HTTP] PUT Response: Status " + std::to_string(response.status_code));
            LogMatrixDebug("[HTTP] Response Body: " + response.text);
            LogMatrixDebug("[HTTP] ===================================");
            
            return {
                static_cast<int>(response.status_code),
                response.text,
                response.status_code >= 200 && response.status_code < 300
            };
        }
        catch (const std::exception& e)
        {
            LogMatrixDebug("[HTTP] PUT Exception: " + std::string(e.what()));
            return { 0, "HTTP Error: " + std::string(e.what()), false };
        }
    }

    std::string UrlEncode(const std::string& input)
    {
        try 
        {
            return std::string(cpr::util::urlEncode(input));
        }
        catch (const std::exception&)
        {
            return input;
        }
    }

    // Set logging callback
    void HttpClient::SetLogCallback(void(*callback)(const std::string&, const std::string&)) {
        g_LogCallback = callback;
    }
}