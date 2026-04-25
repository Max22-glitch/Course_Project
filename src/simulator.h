#pragma once
#include <cstddef>
#include <string>
#include <vector>
#include "announcement.h"
#include "output.h"

struct TargetRoute {
    std::string prefix;
    std::vector<int> as_path;
    int next_hop_asn = 0;
    Relationship received_from = Relationship::ORIGIN;
    bool rov_invalid = false;
};

struct SimulationRequest {
    std::string caida_data;
    std::string rov_csv;
    std::string announcements_csv;
    int target_asn = -1;
};

struct SimulationResponse {
    std::vector<RibEntry> ribs;
    std::vector<TargetRoute> target_routes;
    std::vector<std::string> warnings;
    std::size_t total_ases = 0;
    std::size_t total_routes = 0;
};

SimulationResponse run_simulation(const SimulationRequest& request);
std::string relationship_to_string(Relationship relationship);
