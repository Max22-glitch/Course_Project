#pragma once
#include <string>
#include <vector>
//This file defines the routing information that occurs when one AS announces a prefix to another AS
enum class Relationship {//an enum is a user-defined data type used to assign names to a set of integer constants
    ORIGIN,     
    CUSTOMER,   
    PEER,
    PROVIDER
};
//defines object
struct Announcement {
    std::string prefix;     //this is the IP prefix -> destination block of addresses the route is for
    std::vector<int> as_path;       //this is the path taken so far -> records of the ASNs the announcement has passed through
    int next_hop_asn = 0;       // stores the ASN that sent the announcement most recently
    Relationship received_from = Relationship::ORIGIN;  //relationship type 
    bool rov_invalid = false;   // whether the announcement is marked invalid under ROV
};