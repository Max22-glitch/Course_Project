#pragma once
#include <string>
#include <vector>

class RovParser {
public:
    static std::vector<int> parse_rov_asns(const std::string& filename);
};