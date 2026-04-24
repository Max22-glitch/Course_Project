#include <iostream>
#include <vector>
#include "graph.h"
#include "parser.h"
#include "announcement.h"
#include "announcement_parser.h"
#include "rov_parser.h"
#include "output.h"

int main() {    //this runs the entire program
    Graph graph;

    Parser::parse_caida("bench/subprefix/CAIDAASGraphCollector_2025.10.16.txt", graph);

    if (graph.has_provider_cycle()) {
        std::cout << "Provider cycle detected\n";
        return 1;
    }

    graph.flatten_graph();

    std::vector<int> rov_asns =
        RovParser::parse_rov_asns("bench/subprefix/rov_asns.csv");
    graph.mark_rov_asns(rov_asns);

    graph.init_policies();

    auto seeds = AnnouncementParser::parse_announcements("bench/subprefix/anns.csv");

    for (const auto& seed : seeds) {
        if (!graph.has_as(seed.seed_asn)) {
            std::cout << "Skipping missing ASN: " << seed.seed_asn << "\n";
            continue;
        }

        Announcement ann;
        ann.prefix = seed.prefix;
        ann.as_path = {seed.seed_asn};
        ann.next_hop_asn = seed.seed_asn;
        ann.received_from = Relationship::ORIGIN;
        ann.rov_invalid = seed.rov_invalid;

        graph.propagate_announcement(seed.seed_asn, ann);
    }

    Output::write_ribs("ribs.csv", graph);

    std::cout << "Wrote ribs.csv\n";
    return 0;
}