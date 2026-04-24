#include "rov_parser.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cctype>

static std::string trim(std::string s) {       //this function removes leading and trailing whitespaces froma string
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) {
        s.pop_back();
    }

    size_t start = 0;
    while (start < s.size() &&
           std::isspace(static_cast<unsigned char>(s[start]))) {
        ++start;
    }

    return s.substr(start);
}

static bool is_integer_line(const std::string& s) { //this checks whether the line contains a valid integer
    if (s.empty()) {
        return false;
    }

    size_t start = (s[0] == '-' || s[0] == '+') ? 1 : 0;
    if (start == s.size()) {
        return false;
    }

    for (size_t i = start; i < s.size(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(s[i]))) {
            return false;
        }
    }

    return true;
}

std::vector<int> RovParser::parse_rov_asns(const std::string& filename) {   //this function reads the list of ASNs that use ROV and returns them
    std::ifstream file(filename);

    if (!file.is_open()) {
        throw std::runtime_error("Could not open ROV file: " + filename);
    }

    std::vector<int> result;
    std::string line;

    while (std::getline(file, line)) {
        line = trim(line);

        if (line.empty()) {
            continue;
        }

        if (!is_integer_line(line)) {
            continue;
        }

        std::stringstream ss(line);
        std::string asn_str;

        if (!std::getline(ss, asn_str, ',')) {
            continue;
        }

        result.push_back(std::stoi(asn_str));
    }

    return result;
}
