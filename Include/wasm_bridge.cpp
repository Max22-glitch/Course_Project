#include "simulator.h"
#include <emscripten/emscripten.h>
#include <exception>
#include <string>

static std::string last_ribs_csv;

static std::string escape_json(const std::string& input) {
    std::string output;
    output.reserve(input.size());

    for (char ch : input) {
        switch (ch) {
            case '\\':
                output += "\\\\";
                break;
            case '"':
                output += "\\\"";
                break;
            case '\n':
                output += "\\n";
                break;
            case '\r':
                output += "\\r";
                break;
            case '\t':
                output += "\\t";
                break;
            default:
                output += ch;
                break;
        }
    }

    return output;
}

static std::string path_json(const std::vector<int>& path) {
    std::string json = "[";

    for (std::size_t i = 0; i < path.size(); ++i) {
        if (i > 0) {
            json += ",";
        }

        json += std::to_string(path[i]);
    }

    json += "]";
    return json;
}

extern "C" {
EMSCRIPTEN_KEEPALIVE
const char* run_simulation_json(const char* caida_data,
                                const char* rov_csv,
                                const char* announcements_csv,
                                int target_asn) {
    static std::string response_json;

    try {
        SimulationRequest request;
        request.caida_data = caida_data ? caida_data : "";
        request.rov_csv = rov_csv ? rov_csv : "";
        request.announcements_csv = announcements_csv ? announcements_csv : "";
        request.target_asn = target_asn;

        SimulationResponse response = run_simulation(request);
        last_ribs_csv = "asn,prefix,as_path\n";

        for (const auto& entry : response.ribs) {
            last_ribs_csv += std::to_string(entry.asn);
            last_ribs_csv += ",";
            last_ribs_csv += entry.prefix;
            last_ribs_csv += ",";
            last_ribs_csv += Output::path_to_string(entry.as_path);
            last_ribs_csv += "\n";
        }

        response_json = "{";
        response_json += "\"ok\":true,";
        response_json += "\"total_ases\":" + std::to_string(response.total_ases) + ",";
        response_json += "\"total_routes\":" + std::to_string(response.total_routes) + ",";
        response_json += "\"warnings\":[";

        for (std::size_t i = 0; i < response.warnings.size(); ++i) {
            if (i > 0) {
                response_json += ",";
            }

            response_json += "\"" + escape_json(response.warnings[i]) + "\"";
        }

        response_json += "],";
        response_json += "\"target_routes\":[";

        for (std::size_t i = 0; i < response.target_routes.size(); ++i) {
            if (i > 0) {
                response_json += ",";
            }

            const TargetRoute& route = response.target_routes[i];
            response_json += "{";
            response_json += "\"prefix\":\"" + escape_json(route.prefix) + "\",";
            response_json += "\"as_path\":" + path_json(route.as_path) + ",";
            response_json += "\"as_path_display\":\"" + escape_json(Output::path_to_string(route.as_path)) + "\",";
            response_json += "\"next_hop_asn\":" + std::to_string(route.next_hop_asn) + ",";
            response_json += "\"received_from\":\"" + relationship_to_string(route.received_from) + "\",";
            response_json += std::string("\"rov_invalid\":") + (route.rov_invalid ? "true" : "false");
            response_json += "}";
        }

        response_json += "}";
        return response_json.c_str();
    } catch (const std::exception& error) {
        response_json = "{";
        response_json += "\"ok\":false,";
        response_json += "\"error\":\"" + escape_json(error.what()) + "\"";
        response_json += "}";
        return response_json.c_str();
    }
}

EMSCRIPTEN_KEEPALIVE
const char* get_last_ribs_csv() {
    return last_ribs_csv.c_str();
}
}
