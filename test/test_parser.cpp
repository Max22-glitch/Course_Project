#include <cassert>
#include <fstream>
#include <iostream>
#include "parser.h"
#include "graph.h"

int main() {
    const std::string filename = "test_caida.txt";

    {
        std::ofstream file(filename);
        file << "# comment line\n";
        file << "1|2|-1\n";  // 1 provider of 2
        file << "3|4|0\n";   // peers
        file << "6|5|1\n";   // means 5 provider of 6
    }

    Graph g;
    Parser::parse_caida(filename, g);

    assert(g.has_as(1));
    assert(g.has_as(2));
    assert(g.has_as(3));
    assert(g.has_as(4));
    assert(g.has_as(5));
    assert(g.has_as(6));

    const AS& as1 = g.get_as(1);
    const AS& as2 = g.get_as(2);
    const AS& as3 = g.get_as(3);
    const AS& as4 = g.get_as(4);
    const AS& as5 = g.get_as(5);
    const AS& as6 = g.get_as(6);

    assert(as1.customers.size() == 1 && as1.customers[0] == 2);
    assert(as2.providers.size() == 1 && as2.providers[0] == 1);

    assert(as3.peers.size() == 1 && as3.peers[0] == 4);
    assert(as4.peers.size() == 1 && as4.peers[0] == 3);

    assert(as5.customers.size() == 1 && as5.customers[0] == 6);
    assert(as6.providers.size() == 1 && as6.providers[0] == 5);

    std::cout << "test_parser passed\n";
    return 0;
}
