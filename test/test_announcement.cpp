#include <cassert>
#include <iostream>
#include "announcement.h"

int main() {
    Announcement ann;
    ann.prefix = "1.2.3.0/24";
    ann.as_path = {25, 2152};
    ann.next_hop_asn = 2152;
    ann.received_from = Relationship::PEER;
    ann.rov_invalid = true;

    assert(ann.prefix == "1.2.3.0/24");
    assert(ann.as_path.size() == 2);
    assert(ann.as_path[0] == 25);
    assert(ann.as_path[1] == 2152);
    assert(ann.next_hop_asn == 2152);
    assert(ann.received_from == Relationship::PEER);
    assert(ann.rov_invalid == true);

    std::cout << "test_announcement passed\n";
    return 0;
}
