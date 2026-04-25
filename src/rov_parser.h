#pragma once
#include <string>
#include <vector>

class RovParser {
public:
    static std::vector<int> parse_rov_asns(const std::string& filename);
    static std::vector<int> parse_rov_asns_text(const std::string& contents);
};
