#include "announcement_parser.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cctype>

static std::string trim(std::string s) {    //this function removes leading and trailing whitespace froma string
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

static std::string to_upper(std::string s) {        //this function converts the string to uppercase
    for (char& c : s) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    return s;
}

static std::vector<AnnouncementSeed> parse_announcements_stream(std::istream& input) {
    std::vector<AnnouncementSeed> announcements;
    std::string line;

    std::getline(input, line); // skip header

    while (std::getline(input, line)) {
        if (line.empty()) {
            continue;
        }

        std::stringstream ss(line);
        std::string seed_asn_str, prefix, rov_str;

        std::getline(ss, seed_asn_str, ',');
        std::getline(ss, prefix, ',');
        std::getline(ss, rov_str, ',');

        AnnouncementSeed ann;
        ann.seed_asn = std::stoi(seed_asn_str);
        ann.prefix = trim(prefix);

        rov_str = to_upper(trim(rov_str));
        ann.rov_invalid = (rov_str == "TRUE" || rov_str == "1");

        announcements.push_back(ann);
    }

    return announcements;
}

std::vector<AnnouncementSeed> AnnouncementParser::parse_announcements(const std::string& filename) {        //this function reads the seeded announcements from the given file and returns them as a vector
    std::ifstream file(filename);

    if (!file.is_open()) {
        throw std::runtime_error("Could not open announcements file: " + filename);
    }

    return parse_announcements_stream(file);
}

std::vector<AnnouncementSeed> AnnouncementParser::parse_announcements_text(const std::string& contents) {
    std::istringstream input(contents);
    return parse_announcements_stream(input);
}
