#include "simulator.h"
#include <algorithm>
#include <stdexcept>
#include "announcement_parser.h"
#include "graph.h"
#include "parser.h"
#include "rov_parser.h"

std::string relationship_to_string(Relationship relationship) {
    switch (relationship) {
        case Relationship::ORIGIN:
            return "ORIGIN";
        case Relationship::CUSTOMER:
            return "CUSTOMER";
        case Relationship::PEER:
            return "PEER";
        case Relationship::PROVIDER:
            return "PROVIDER";
    }

    return "UNKNOWN";
}

SimulationResponse run_simulation(const SimulationRequest& request) {
    Graph graph;
    Parser::parse_caida_text(request.caida_data, graph);

    if (graph.has_provider_cycle()) {
        throw std::runtime_error("Provider cycle detected");
    }

    graph.flatten_graph();
    graph.mark_rov_asns(RovParser::parse_rov_asns_text(request.rov_csv));
    graph.init_policies();

    auto seeds = AnnouncementParser::parse_announcements_text(request.announcements_csv);

    for (const auto& seed : seeds) {
        if (!graph.has_as(seed.seed_asn)) {
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

    SimulationResponse response;
    response.total_ases = graph.get_all().size();

    for (const auto& pair : graph.get_all()) {
        int asn = pair.first;
        const AS& node = pair.second;

        if (!node.policy) {
            continue;
        }

        const auto& rib = node.policy->get_local_rib();

        for (const auto& entry : rib) {
            response.ribs.push_back(RibEntry{asn, entry.first, entry.second.as_path});

            if (asn == request.target_asn) {
                response.target_routes.push_back(
                    TargetRoute{
                        entry.first,
                        entry.second.as_path,
                        entry.second.next_hop_asn,
                        entry.second.received_from,
                        entry.second.rov_invalid
                    }
                );
            }
        }
    }

    std::sort(response.ribs.begin(), response.ribs.end(), [](const RibEntry& left, const RibEntry& right) {
        if (left.asn != right.asn) {
            return left.asn < right.asn;
        }

        if (left.prefix != right.prefix) {
            return left.prefix < right.prefix;
        }

        return left.as_path < right.as_path;
    });

    std::sort(response.target_routes.begin(), response.target_routes.end(), [](const TargetRoute& left, const TargetRoute& right) {
        if (left.prefix != right.prefix) {
            return left.prefix < right.prefix;
        }

        return left.as_path < right.as_path;
    });

    response.total_routes = response.ribs.size();

    if (!graph.has_as(request.target_asn)) {
        response.warnings.push_back("Target ASN is not present in the topology.");
    } else if (response.target_routes.empty()) {
        response.warnings.push_back("Target ASN received no routes for the uploaded announcements.");
    }

    return response;
}
