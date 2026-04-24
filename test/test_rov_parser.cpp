#include <cassert>
#include <fstream>
#include <iostream>
#include "rov_parser.h"

int main() {
    const std::string filename = "test_rov.csv";

    {
        std::ofstream file(filename);
        file << "asn\n";
        file << "25\n";
        file << "27\n";
        file << "42\n";
    }

    auto rov = RovParser::parse_rov_asns(filename);

    assert(rov.size() == 3);
    assert(rov[0] == 25);
    assert(rov[1] == 27);
    assert(rov[2] == 42);

    std::cout << "test_rov_parser passed\n";
    return 0;
}
