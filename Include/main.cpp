#include <iostream>
#include <fstream>
#include <sstream>
#include "output.h"
#include "simulator.h"

static std::string read_file(const std::string& filename) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main(int argc, char** argv) {    //this runs the entire program
    const std::string caida_path =
        argc > 1 ? argv[1] : "bench/subprefix/CAIDAASGraphCollector_2025.10.16.txt";
    const std::string rov_path =
        argc > 2 ? argv[2] : "bench/subprefix/rov_asns.csv";
    const std::string anns_path =
        argc > 3 ? argv[3] : "bench/subprefix/anns.csv";
    const std::string output_path =
        argc > 4 ? argv[4] : "ribs.csv";

    SimulationRequest request;
    request.caida_data = read_file(caida_path);
    request.rov_csv = read_file(rov_path);
    request.announcements_csv = read_file(anns_path);

    SimulationResponse response = run_simulation(request);
    Output::write_ribs(output_path, response.ribs);

    std::cout << "Wrote " << output_path << "\n";
    return 0;
}
