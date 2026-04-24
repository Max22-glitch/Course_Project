#include <cassert>
#include <iostream>
#include "graph.h"
#include "bgp.h"
#include "announcement.h"

int main() {
    Graph g;

    // 1 provider of 2, 2 provider of 3
    g.add_provider_customer(1, 2);
    g.add_provider_customer(2, 3);

    g.init_policies();

    Announcement ann;
    ann.prefix = "1.2.0.0/16";
    ann.as_path = {3};
    ann.next_hop_asn = 3;
    ann.received_from = Relationship::ORIGIN;
    ann.rov_invalid = false;

    g.propagate_announcement(3, ann);

    const Announcement* path3 = g.get_as(3).policy->get_best("1.2.0.0/16");
    const Announcement* path2 = g.get_as(2).policy->get_best("1.2.0.0/16");
    const Announcement* path1 = g.get_as(1).policy->get_best("1.2.0.0/16");

    assert(path3 != nullptr);
    assert(path2 != nullptr);
    assert(path1 != nullptr);

    assert(path3->as_path.size() == 1);
    assert(path3->as_path[0] == 3);

    assert(path2->as_path.size() == 2);
    assert(path1->as_path.size() == 3);

    std::cout << "AS3 path: ";
    for (int x : path3->as_path) std::cout << x << " ";
    std::cout << "\n";

    std::cout << "AS2 path: ";
    for (int x : path2->as_path) std::cout << x << " ";
    std::cout << "\n";

    std::cout << "AS1 path: ";
    for (int x : path1->as_path) std::cout << x << " ";
    std::cout << "\n";

    std::cout << "test_graph_propagation passed\n";
    return 0;
}
