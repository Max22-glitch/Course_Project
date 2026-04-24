#include "bgp.h"

BGP::BGP(int asn, bool uses_rov)//This is the constructor for the BGP class
    : asn_(asn), uses_rov_(uses_rov) {} //initializer list

void BGP::receive(const Announcement& ann) {    //when the graph sends the announcement to the AS’s policy this function decides what to do with it
    if (uses_rov_ && ann.rov_invalid) {
        return;
    }

    received_[ann.prefix].push_back(ann);
}

int BGP::relationship_value(Relationship r) const { //assigns values to different relationships
    if (r == Relationship::CUSTOMER) return 3;
    if (r == Relationship::PEER) return 2;
    if (r == Relationship::PROVIDER) return 1;
    return 4;
}

bool BGP::better(const Announcement& a, const Announcement& b) const {  //choses the better route
    int a_rel = relationship_value(a.received_from);
    int b_rel = relationship_value(b.received_from);

    if (a_rel != b_rel) {   //choses the superior connection
        return a_rel > b_rel;
    }

    if (a.as_path.size() != b.as_path.size()) {     //chooses shorter path if type of relationship is the same
        return a.as_path.size() < b.as_path.size();
    }

    return a.next_hop_asn < b.next_hop_asn; //if the realtionship and size are the same it chooses based on next hop
}

const Announcement* BGP::get_best(const std::string& prefix) const {    //returns current best route
    auto it = local_rib_.find(prefix);
    if (it != local_rib_.end()) {
        return &it->second;
    }
    return nullptr;
}

const std::unordered_map<std::string, Announcement>& BGP::get_local_rib() const {   //returns local routing table
    return local_rib_;
}

bool BGP::process() {   //This function goes through all newly received announcements, compares them, decides the best route for each prefix, and updates the local RIB
    bool changed = false;   //tracks RIB change during procossing 

    for (auto& pair : received_) {  // loop goes through each prefix that has incoming routes
        const std::string& prefix = pair.first;
        std::vector<Announcement>& anns = pair.second;

        if (anns.empty()) {
            continue;
        }

        Announcement best;
        auto rib_it = local_rib_.find(prefix);

        if (rib_it != local_rib_.end()) {
            best = rib_it->second;
        } else {
            best = anns[0];
        }

        for (const auto& ann : anns) {  //function loops through every newly received announcement for this prefix
            if (better(ann, best)) {
                best = ann;
            }
        }

        auto it = local_rib_.find(prefix);
        if (it == local_rib_.end()) {
            local_rib_[prefix] = best;
            changed = true;
        } else {
            bool different =
                best.as_path != it->second.as_path ||
                best.next_hop_asn != it->second.next_hop_asn ||
                best.received_from != it->second.received_from ||
                best.rov_invalid != it->second.rov_invalid;

            if (different) {
                local_rib_[prefix] = best;
                changed = true;
            }
        }
    }

    received_.clear();
    return changed;
}
