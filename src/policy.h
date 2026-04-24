#pragma once
#include <string>
#include <unordered_map>
#include "announcement.h"
//policy is the set of rules the AS uses when an announcement arrives
//so policy.h defines the idea of what a routing policy must be able to do
class Policy {
public:
    virtual bool process() = 0;     //looks through announcments and decides on best route
    virtual void receive(const Announcement& ann) = 0;  // gives the policy a new announcement
    virtual const Announcement* get_best(const std::string& prefix) const = 0;  //returns the current best route for a given prefix
    virtual const std::unordered_map<std::string, Announcement>& get_local_rib() const = 0; // returns the entire local routing table

    virtual ~Policy() = default;    //deconstructor
};