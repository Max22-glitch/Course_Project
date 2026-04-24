#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include "policy.h"
//defines the BGP class, which is the routing policy class used by each AS
class BGP : public Policy {     //BGP inherits from Policy and implements its functions
private:
    int asn_;   //stores the ASN of the AS
    bool uses_rov_; //This tells whether this AS uses ROV

    std::unordered_map<std::string, Announcement> local_rib_;   //local routing table which maps: prefix -> best announcement
    std::unordered_map<std::string, std::vector<Announcement>> received_; //stores incoming announcements that have been received but not fully processed

    bool better(const Announcement& a, const Announcement& b) const;    //Compares two announcements and returns the superior one
    int relationship_value(Relationship r) const;   //Convert a relationship type into a number for comparisons 

public:
    BGP(int asn, bool uses_rov);

    void receive(const Announcement& ann) override; //implements the Policy function receive
    bool process() override;   //implements the Policy fucntion process
    const Announcement* get_best(const std::string& prefix) const override; //Returns the current best route for a given prefix
    const std::unordered_map<std::string, Announcement>& get_local_rib() const override;
};