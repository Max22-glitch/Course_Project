#include <cassert>
#include <fstream>
#include <iostream>
#include "announcement_parser.h"

int main() {
    const std::string filename = "test_anns.csv";

    {
        std::ofstream file(filename);
        file << "seed_asn,prefix,rov_invalid\n";
        file << "27,1.2.0.0/16,TRUE\n";
        file << "25,1.2.3.0/24,FALSE\n";
        file << "42,9.9.9.0/24,1\n";
    }

    auto anns = AnnouncementParser::parse_announcements(filename);

    assert(anns.size() == 3);

    assert(anns[0].seed_asn == 27);
    assert(anns[0].prefix == "1.2.0.0/16");
    assert(anns[0].rov_invalid == true);

    assert(anns[1].seed_asn == 25);
    assert(anns[1].prefix == "1.2.3.0/24");
    assert(anns[1].rov_invalid == false);

    assert(anns[2].seed_asn == 42);
    assert(anns[2].prefix == "9.9.9.0/24");
    assert(anns[2].rov_invalid == true);

    std::cout << "test_announcement_parser passed\n";
    return 0;
}
