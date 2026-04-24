#include "output.h"
#include <fstream>
#include <stdexcept>

static std::string path_to_string(const std::vector<int>& path) {   //this function converts the AS path vector into a string for outputting into the CSV file
    std::string result = "\"(";

    for (size_t i = 0; i < path.size(); ++i) {
        result += std::to_string(path[i]);
        if (i + 1 < path.size()) {
            result += ", ";
        }
    }

    if (path.size() == 1) {
        result += ",";
    }

    result += ")\"";
    return result;
}

void Output::write_ribs(const std::string& filename, const Graph& graph) {  //this writes the final RIBs of each AS into the CSV file
    std::ofstream file(filename);

    if (!file.is_open()) {
        throw std::runtime_error("Could not open output file");
    }

    file << "asn,prefix,as_path\n";

    for (const auto& pair : graph.get_all()) {
        int asn = pair.first;
        const AS& node = pair.second;

        if (!node.policy) {
            continue;
        }

        const auto& rib = node.policy->get_local_rib();

        for (const auto& entry : rib) {
            file << asn << ","
                 << entry.first << ","
                 << path_to_string(entry.second.as_path)
                 << "\n";
        }
    }
}
