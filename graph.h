#pragma once
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include "policy.h"
#include "announcement.h"
// this file defines the main network structure by creating the graph it will be routing through
struct AS { 
    int asn = 0;
    std::vector<int> providers;
    std::vector<int> customers;
    std::vector<int> peers;
    int rank = -1;
    bool uses_rov = false;

    std::unique_ptr<Policy> policy;
};

class Graph {   //Graph class stores the full AS graph and provides the main operations on it
private:
    std::unordered_map<int, AS> nodes_;

    bool dfs_provider_cycle(int asn,
                            std::unordered_map<int, int>& state) const;

    void send_to_neighbors(int from_asn, const Announcement& ann);

    bool can_export_to_customer(const Announcement& ann) const;
    bool can_export_to_provider(const Announcement& ann) const;
    bool can_export_to_peer(const Announcement& ann) const;
    bool path_contains_asn(const Announcement& ann, int asn) const;

public: //node access functions
    AS& get_or_create(int asn);
    bool has_as(int asn) const;
    AS& get_as(int asn);
    const AS& get_as(int asn) const;                            

    void add_provider_customer(int provider, int customer); 
    void add_peer(int as1, int as2);

    const std::unordered_map<int, AS>& get_all() const;   
    void reserve(size_t n);

    bool has_provider_cycle() const;
    std::vector<std::vector<int>> flatten_graph();

    void mark_rov_asns(const std::vector<int>& rov_asns);
    void init_policies();

    void propagate_announcement(int origin_asn, const Announcement& ann);
    void print_best_path(int asn, const std::string& prefix) const;
};