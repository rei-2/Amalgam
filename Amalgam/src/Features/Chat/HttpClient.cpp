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

namespace HttpClient 
{
    HttpResponse Get(const std::string& url, const std::map<std::string, std::string>& headers)
    {
        try 
        {
            // Log request details
            printf("[Matrix HTTP] GET Request: %s\n", url.c_str());
            printf("[Matrix HTTP] Headers:\n");
            for (const auto& header : headers) 
            {
                // Don't log sensitive headers in full
                if (header.first == "Authorization") {
                    printf("  %s: Bearer [REDACTED]\n", header.first.c_str());
                } else {
                    printf("  %s: %s\n", header.first.c_str(), header.second.c_str());
                }
            }
            
            cpr::Header cprHeaders;
            for (const auto& header : headers) 
            {
                cprHeaders[header.first] = header.second;
            }

            auto response = cpr::Get(cpr::Url{url}, cprHeaders, cpr::VerifySsl{false});
            
            // Log response details
            printf("[Matrix HTTP] GET Response: Status %ld\n", response.status_code);
            printf("[Matrix HTTP] Response Body: %s\n", response.text.c_str());
            printf("[Matrix HTTP] ===================================\n");
            
            return {
                static_cast<int>(response.status_code),
                response.text,
                response.status_code >= 200 && response.status_code < 300
            };
        }
        catch (const std::exception& e)
        {
            printf("[Matrix HTTP] GET Exception: %s\n", e.what());
            return { 0, "HTTP Error: " + std::string(e.what()), false };
        }
    }

    HttpResponse Post(const std::string& url, const std::string& body, const std::map<std::string, std::string>& headers)
    {
        try 
        {
            // Log request details
            printf("[Matrix HTTP] POST Request: %s\n", url.c_str());
            printf("[Matrix HTTP] Headers:\n");
            for (const auto& header : headers) 
            {
                // Don't log sensitive headers in full
                if (header.first == "Authorization") {
                    printf("  %s: Bearer [REDACTED]\n", header.first.c_str());
                } else {
                    printf("  %s: %s\n", header.first.c_str(), header.second.c_str());
                }
            }
            printf("[Matrix HTTP] Request Body: %s\n", body.c_str());
            
            cpr::Header cprHeaders;
            for (const auto& header : headers) 
            {
                cprHeaders[header.first] = header.second;
            }

            auto response = cpr::Post(cpr::Url{url}, cpr::Body{body}, cprHeaders, cpr::VerifySsl{false});
            
            // Log response details
            printf("[Matrix HTTP] POST Response: Status %ld\n", response.status_code);
            printf("[Matrix HTTP] Response Body: %s\n", response.text.c_str());
            printf("[Matrix HTTP] ===================================\n");
            
            return {
                static_cast<int>(response.status_code),
                response.text,
                response.status_code >= 200 && response.status_code < 300
            };
        }
        catch (const std::exception& e)
        {
            printf("[Matrix HTTP] POST Exception: %s\n", e.what());
            return { 0, "HTTP Error: " + std::string(e.what()), false };
        }
    }

    HttpResponse Put(const std::string& url, const std::string& body, const std::map<std::string, std::string>& headers)
    {
        try 
        {
            // Log request details
            printf("[Matrix HTTP] PUT Request: %s\n", url.c_str());
            printf("[Matrix HTTP] Headers:\n");
            for (const auto& header : headers) 
            {
                // Don't log sensitive headers in full
                if (header.first == "Authorization") {
                    printf("  %s: Bearer [REDACTED]\n", header.first.c_str());
                } else {
                    printf("  %s: %s\n", header.first.c_str(), header.second.c_str());
                }
            }
            printf("[Matrix HTTP] Request Body: %s\n", body.c_str());
            
            cpr::Header cprHeaders;
            for (const auto& header : headers) 
            {
                cprHeaders[header.first] = header.second;
            }

            auto response = cpr::Put(cpr::Url{url}, cpr::Body{body}, cprHeaders, cpr::VerifySsl{false});
            
            // Log response details
            printf("[Matrix HTTP] PUT Response: Status %ld\n", response.status_code);
            printf("[Matrix HTTP] Response Body: %s\n", response.text.c_str());
            printf("[Matrix HTTP] ===================================\n");
            
            return {
                static_cast<int>(response.status_code),
                response.text,
                response.status_code >= 200 && response.status_code < 300
            };
        }
        catch (const std::exception& e)
        {
            printf("[Matrix HTTP] PUT Exception: %s\n", e.what());
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
}