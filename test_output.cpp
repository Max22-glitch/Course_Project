#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include "graph.h"
#include "bgp.h"
#include "output.h"

int main() {
    Graph g;
    g.add_provider_customer(1, 2);
    g.init_policies();

    Announcement ann;
    ann.prefix = "1.2.0.0/16";
    ann.as_path = {2};
    ann.next_hop_asn = 2;
    ann.received_from = Relationship::ORIGIN;

    g.propagate_announcement(2, ann);

    Output::write_ribs("test_ribs.csv", g);

    std::ifstream file("test_ribs.csv");
    assert(file.is_open());

    std::string line;
    std::getline(file, line);
    assert(line == "asn,prefix,as_path");

    bool found_prefix = false;
    while (std::getline(file, line)) {
        if (line.find("1.2.0.0/16") != std::string::npos) {
            found_prefix = true;
        }
    }

    assert(found_prefix == true);

    std::cout << "test_output passed\n";
    return 0;
}
