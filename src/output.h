#pragma once
#include <string>
#include <vector>
#include "graph.h"

struct RibEntry {
    int asn = 0;
    std::string prefix;
    std::vector<int> as_path;
};

class Output {  //this class provides a function to write the final RIBs of each AS into our own CSV file
public:
    static void write_ribs(const std::string& filename, const Graph& graph);
    static void write_ribs(const std::string& filename, const std::vector<RibEntry>& ribs);
    static std::string path_to_string(const std::vector<int>& path);
};
