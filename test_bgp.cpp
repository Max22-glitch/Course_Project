#include <cassert>
#include <iostream>
#include "bgp.h"

int main() {
    BGP bgp(100, false);

    Announcement a;
    a.prefix = "1.2.0.0/16";
    a.as_path = {200, 300};
    a.next_hop_asn = 200;
    a.received_from = Relationship::PROVIDER;

    Announcement b;
    b.prefix = "1.2.0.0/16";
    b.as_path = {400};
    b.next_hop_asn = 400;
    b.received_from = Relationship::CUSTOMER;

    bgp.receive(a);
    bgp.receive(b);

    bool changed = bgp.process();
    assert(changed == true);

    const Announcement* best = bgp.get_best("1.2.0.0/16");
    assert(best != nullptr);
    assert(best->received_from == Relationship::CUSTOMER);
    assert(best->next_hop_asn == 400);
    assert(best->as_path.size() == 1);
    assert(best->as_path[0] == 400);

    // test ROV filtering
    BGP rov_bgp(200, true);

    Announcement invalid;
    invalid.prefix = "9.9.9.0/24";
    invalid.as_path = {999};
    invalid.next_hop_asn = 999;
    invalid.received_from = Relationship::CUSTOMER;
    invalid.rov_invalid = true;

    rov_bgp.receive(invalid);
    bool rov_changed = rov_bgp.process();
    assert(rov_changed == false);
    assert(rov_bgp.get_best("9.9.9.0/24") == nullptr);

    std::cout << "test_bgp passed\n";
    return 0;
}
