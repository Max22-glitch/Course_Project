#pragma once
#include <string>
#include <vector>

struct AnnouncementSeed {
    int seed_asn = 0;
    std::string prefix;
    bool rov_invalid = false;
};

class AnnouncementParser {
public:
    static std::vector<AnnouncementSeed> parse_announcements(const std::string& filename);
    static std::vector<AnnouncementSeed> parse_announcements_text(const std::string& contents);
};
