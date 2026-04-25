#include "graph.h"
#include "bgp.h"
#include <queue>
#include <algorithm>
#include <iostream>

AS& Graph::get_or_create(int asn) {     //Get the AS with this ASN if it already exists, or create it if it doesn’t
    return nodes_.try_emplace(asn, AS{asn}).first->second;
}

bool Graph::has_as(int asn) const {
    return nodes_.find(asn) != nodes_.end();
}

AS& Graph::get_as(int asn) {
    return nodes_.at(asn);
}

const AS& Graph::get_as(int asn) const {
    return nodes_.at(asn);
}

void Graph::add_provider_customer(int provider, int customer) {
    AS& provider_as = get_or_create(provider);
    AS& customer_as = get_or_create(customer);

    provider_as.customers.push_back(customer);
    customer_as.providers.push_back(provider);
}

void Graph::add_peer(int as1, int as2) {
    AS& first = get_or_create(as1);
    AS& second = get_or_create(as2);

    first.peers.push_back(as2);
    second.peers.push_back(as1);
}

const std::unordered_map<int, AS>& Graph::get_all() const {
    return nodes_;
}

void Graph::reserve(size_t n) {
    nodes_.reserve(n);
}

bool Graph::has_provider_cycle() const {    //Check whether the graph contains a provider cycle by prreforming dfs on the provider relationships
    std::unordered_map<int, int> state;
    std::vector<std::pair<int, std::size_t>> stack;

    for (const auto& pair : nodes_) {
        const int root_asn = pair.first;

        if (state[root_asn] != 0) {
            continue;
        }

        stack.clear();
        stack.push_back({root_asn, 0});
        state[root_asn] = 1;

        while (!stack.empty()) {
            auto& frame = stack.back();
            const AS& node = nodes_.at(frame.first);

            if (frame.second >= node.providers.size()) {
                state[frame.first] = 2;
                stack.pop_back();
                continue;
            }

            const int provider = node.providers[frame.second++];

            if (state[provider] == 1) {
                return true;
            }

            if (state[provider] == 0) {
                state[provider] = 1;
                stack.push_back({provider, 0});
            }
        }
    }

    return false;
}

std::vector<std::vector<int>> Graph::flatten_graph() {  //Assign ranks to ASes and organize them into propagation layers so the announcments propogate in order
    std::queue<int> q;
    int max_rank = 0;

    for (auto& pair : nodes_) {
        pair.second.rank = -1;
    }

    for (auto& pair : nodes_) {
        AS& node = pair.second;
        if (node.customers.empty()) {
            node.rank = 0;
            q.push(node.asn);
        }
    }

    while (!q.empty()) {
        int current_asn = q.front();
        q.pop();

        AS& current = nodes_.at(current_asn);

        for (int provider_asn : current.providers) {
            AS& provider = nodes_.at(provider_asn);

            if (provider.rank < current.rank + 1) {
                provider.rank = current.rank + 1;
                max_rank = std::max(max_rank, provider.rank);
                q.push(provider_asn);
            }
        }
    }

    std::vector<std::vector<int>> ranks(max_rank + 1);

    for (const auto& pair : nodes_) {
        const AS& node = pair.second;
        if (node.rank >= 0) {
            ranks[node.rank].push_back(node.asn);
        }
    }

    // deterministic order inside each rank group
    for (auto& group : ranks) {
        std::sort(group.begin(), group.end(), std::greater<int>());
    }

    return ranks;
}

void Graph::mark_rov_asns(const std::vector<int>& rov_asns) {   //Marks ASes using ROV.
    for (int asn : rov_asns) {
        if (has_as(asn)) {
            nodes_.at(asn).uses_rov = true;
        }
    }
}

void Graph::init_policies() {   //Create a routing-policy object for every AS
    for (auto& pair : nodes_) {
        int asn = pair.first;
        AS& node = pair.second;
        node.policy = std::make_unique<BGP>(asn, node.uses_rov);
    }
}

bool Graph::path_contains_asn(const Announcement& ann, int asn) const {     //Checks whether an ASN already appears in an announcement’s AS path
    for (int x : ann.as_path) {
        if (x == asn) {
            return true;
        }
    }
    return false;
}
//functions the decide whether an announcement can be exported to a neighbor based on their relationship
bool Graph::can_export_to_customer(const Announcement& ann) const {
    return true;
}

bool Graph::can_export_to_provider(const Announcement& ann) const {
    return ann.received_from == Relationship::ORIGIN ||
           ann.received_from == Relationship::CUSTOMER;
}

bool Graph::can_export_to_peer(const Announcement& ann) const {
    return ann.received_from == Relationship::ORIGIN ||
           ann.received_from == Relationship::CUSTOMER;
}

void Graph::send_to_neighbors(int from_asn, const Announcement& ann) {
    const AS& from = nodes_.at(from_asn);

    if (can_export_to_customer(ann)) {
        for (int customer_asn : from.customers) {
            if (path_contains_asn(ann, customer_asn)) {
                continue;
            }

            Announcement out = ann;
            out.as_path.insert(out.as_path.begin(), customer_asn);
            out.next_hop_asn = from_asn;
            out.received_from = Relationship::PROVIDER;
            nodes_.at(customer_asn).policy->receive(out);
        }
    }

    if (can_export_to_provider(ann)) {
        for (int provider_asn : from.providers) {
            if (path_contains_asn(ann, provider_asn)) {
                continue;
            }

            Announcement out = ann;
            out.as_path.insert(out.as_path.begin(), provider_asn);
            out.next_hop_asn = from_asn;
            out.received_from = Relationship::CUSTOMER;
            nodes_.at(provider_asn).policy->receive(out);
        }
    }

    if (can_export_to_peer(ann)) {
        for (int peer_asn : from.peers) {
            if (path_contains_asn(ann, peer_asn)) {
                continue;
            }

            Announcement out = ann;
            out.as_path.insert(out.as_path.begin(), peer_asn);
            out.next_hop_asn = from_asn;
            out.received_from = Relationship::PEER;
            nodes_.at(peer_asn).policy->receive(out);
        }
    }
}

void Graph::propagate_announcement(int origin_asn, const Announcement& ann) {   //propagates a seeded accouncment through the whole graph.
    if (!has_as(origin_asn)) {
        return;
    }

    std::vector<std::vector<int>> ranks = flatten_graph();
    auto send_to = [&](int from_asn,
                       int to_asn,
                       Relationship received_from,
                       const Announcement& current) {
        if (path_contains_asn(current, to_asn)) {
            return;
        }

        Announcement out = current;
        out.as_path.insert(out.as_path.begin(), to_asn);
        out.next_hop_asn = from_asn;
        out.received_from = received_from;
        nodes_.at(to_asn).policy->receive(out);
    };

    nodes_.at(origin_asn).policy->receive(ann);
    nodes_.at(origin_asn).policy->process();

    for (size_t rank = 0; rank < ranks.size(); ++rank) {
        for (int asn : ranks[rank]) {
            const Announcement* best = nodes_.at(asn).policy->get_best(ann.prefix);
            if (!best) {
                continue;
            }

            for (int provider_asn : nodes_.at(asn).providers) {
                send_to(asn, provider_asn, Relationship::CUSTOMER, *best);
            }
        }

        if (rank + 1 < ranks.size()) {
            for (int asn : ranks[rank + 1]) {
                nodes_.at(asn).policy->process();
            }
        }
    }

    for (const auto& pair : nodes_) {
        int asn = pair.first;
        const Announcement* best = pair.second.policy->get_best(ann.prefix);
        if (!best) {
            continue;
        }

        for (int peer_asn : pair.second.peers) {
            send_to(asn, peer_asn, Relationship::PEER, *best);
        }
    }

    for (auto& pair : nodes_) {
        pair.second.policy->process();
    }

    for (size_t rank = ranks.size(); rank-- > 0;) {
        for (int asn : ranks[rank]) {
            const Announcement* best = nodes_.at(asn).policy->get_best(ann.prefix);
            if (!best) {
                continue;
            }

            for (int customer_asn : nodes_.at(asn).customers) {
                send_to(asn, customer_asn, Relationship::PROVIDER, *best);
            }
        }

        if (rank > 0) {
            for (int asn : ranks[rank - 1]) {
                nodes_.at(asn).policy->process();
            }
        }
    }
}

void Graph::print_best_path(int asn, const std::string& prefix) const { //print function for best path
    const AS& node = nodes_.at(asn);
    const Announcement* best = node.policy->get_best(prefix);

    if (!best) {
        std::cout << "AS" << asn << " has no route for " << prefix << "\n";
        return;
    }

    std::cout << "AS" << asn << " best path for " << prefix << ": ";
    for (int x : best->as_path) {
        std::cout << x << " ";
    }
    std::cout << "\n";
}
