#include "parser.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

static void parse_caida_stream(std::istream& input, Graph& graph) {
    std::string line;

    while (std::getline(input, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::stringstream ss(line);
        std::string a, b, rel;

        std::getline(ss, a, '|');
        std::getline(ss, b, '|');
        std::getline(ss, rel, '|');

        int as1 = std::stoi(a);
        int as2 = std::stoi(b);
        int relationship = std::stoi(rel);

        if (relationship == -1) {
            graph.add_provider_customer(as1, as2);
        }
        else if (relationship == 1) {
            graph.add_provider_customer(as2, as1);
        }
        else if (relationship == 0) {
            graph.add_peer(as1, as2);
        }
    }
}

void Parser::parse_caida(const std::string& filename, Graph& graph) {   //this function reads the AS graph from the CAIDA file and sends it to the graph object
    std::ifstream file(filename);

    if (!file.is_open()) {
        throw std::runtime_error("Could not open CAIDA file: " + filename);
    }

    parse_caida_stream(file, graph);
}

void Parser::parse_caida_text(const std::string& contents, Graph& graph) {
    std::istringstream input(contents);
    parse_caida_stream(input, graph);
}
