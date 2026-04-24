#include <cassert>
#include <iostream>
#include "graph.h"
#include "bgp.h"

int main() {
    Graph g;

    g.add_provider_customer(1, 2);
    g.add_provider_customer(1, 3);
    g.add_provider_customer(4, 1);
    g.add_peer(2, 5);

    assert(g.has_as(1));
    assert(g.has_as(2));
    assert(g.has_as(3));
    assert(g.has_as(4));
    assert(g.has_as(5));

    assert(g.has_provider_cycle() == false);

    auto ranks = g.flatten_graph();

   
    assert(g.get_as(2).rank == 0);
    assert(g.get_as(3).rank == 0);
    assert(g.get_as(5).rank == 0);

    
    assert(g.get_as(1).rank >= 1);

    assert(g.get_as(4).rank > g.get_as(1).rank);

    g.mark_rov_asns({2, 5});
    assert(g.get_as(2).uses_rov == true);
    assert(g.get_as(5).uses_rov == true);
    assert(g.get_as(1).uses_rov == false);

    g.init_policies();
    assert(g.get_as(1).policy != nullptr);
    assert(g.get_as(2).policy != nullptr);

    std::cout << "test_graph_basic passed\n";
    return 0;
}
