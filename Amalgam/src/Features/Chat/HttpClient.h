#pragma once
#include <string>
#include <map>

// Forward declaration to avoid including CPR headers in main codebase
namespace HttpClient 
{
    struct HttpResponse 
    {
        int status_code;
        std::string text;
        bool success;
    };

    // HTTP operations - isolated from CPR
    HttpResponse Get(const std::string& url, const std::map<std::string, std::string>& headers = {});
    HttpResponse Post(const std::string& url, const std::string& body, const std::map<std::string, std::string>& headers = {});
    HttpResponse Put(const std::string& url, const std::string& body, const std::map<std::string, std::string>& headers = {});
    
    // URL encoding utility
    std::string UrlEncode(const std::string& input);
    
    // Set logging callback for HTTP requests
    void SetLogCallback(void(*callback)(const std::string&, const std::string&));
}