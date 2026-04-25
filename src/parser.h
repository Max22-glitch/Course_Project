#pragma once
#include <string>
#include "graph.h"

class Parser { //this class provides a fucntion to read the AS graph from the CAIDA file and send it to the graph object
public:
    static void parse_caida(const std::string& filename, Graph& graph);
    static void parse_caida_text(const std::string& contents, Graph& graph);
};
