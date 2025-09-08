#pragma once

#include <string>
#include <optional>
#include <unordered_map>

// stub structure representing loaded threshold values
struct ContractThresholds {
    std::unordered_map<std::string, double> numeric;
};

class ContractLoader {
public:
    // attempts to parse a thresholds file (markdown or json). Returns nullopt on failure
    static std::optional<ContractThresholds> Load(const std::string& path);
};
