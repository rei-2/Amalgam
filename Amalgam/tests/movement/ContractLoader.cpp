#include "ContractLoader.h"
#include <fstream>
#include <regex>

std::optional<ContractThresholds> ContractLoader::Load(const std::string& path)
{
    std::ifstream ifs(path);
    if (!ifs.is_open()) return std::nullopt;
    ContractThresholds ct{};
    std::string line;
    // Extremely naive markdown table parser: looks for lines with pattern "| Metric | ID |" then pulls threshold column.
    std::regex row(R"(^\|([^|]+)\|([^|]+)\|([^|]+)\|([^|]+)\|([^|]+)\|$)");
    while (std::getline(ifs, line)) {
        std::smatch m;
        if (std::regex_match(line, m, row)) {
            if (m.size() == 6) {
                std::string id = std::string(m[2]);
                std::string threshold = std::string(m[4]);
                // Attempt to extract numeric value (first number in threshold string)
                std::smatch num;
                if (std::regex_search(threshold, num, std::regex(R"([-+]?[0-9]*\.?[0-9]+)"))) {
                    ct.numeric[id] = std::stod(num.str());
                }
            }
        }
    }
    return ct;
}
