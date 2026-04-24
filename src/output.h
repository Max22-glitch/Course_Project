#pragma once
#include <string>
#include "graph.h"

class Output {  //this class provides a function to write the final RIBs of each AS into our own CSV file
public:
    static void write_ribs(const std::string& filename, const Graph& graph);
};