## CSE3150 Course Project: BGP Simulator

This project implements a basic BGP simulator in C++. It builds an AS graph from CAIDA relationship data, seeds BGP announcements, propagates them according to customer/peer/provider policy, applies ROV filtering, and writes the resulting local RIB contents to a CSV file.

## Features

- Parses CAIDA AS relationship data into an AS graph
- Stores provider, customer, and peer relationships for each AS
- Detects provider cycles before propagation
- Flattens the graph into propagation ranks
- Seeds announcements from an announcements CSV
- Propagates announcements using the intended BGP flow:
  - up to providers
  - across one peer hop
  - down to customers
- Selects best routes using:
  - relationship preference: `ORIGIN > CUSTOMER > PEER > PROVIDER`
  - shortest AS path
  - lowest next-hop ASN
- Supports ROV by dropping `rov_invalid` announcements at ROV-enabled ASNs
- Writes final routes to `ribs.csv` with columns `asn`, `prefix`, and `as_path`

## File Overview (On my computer)

- [main.cpp](/Users/maximusformoso/Desktop/CSE3150/Course_Project/main.cpp): loads data, runs propagation, writes output
- [graph.h](/Users/maximusformoso/Desktop/CSE3150/Course_Project/graph.h), [graph.cpp](/Users/maximusformoso/Desktop/CSE3150/Course_Project/graph.cpp): AS graph structure and propagation logic
- [bgp.h](/Users/maximusformoso/Desktop/CSE3150/Course_Project/bgp.h), [bgp.cpp](/Users/maximusformoso/Desktop/CSE3150/Course_Project/bgp.cpp): local RIB, receive queue, and route selection
- [parser.h](/Users/maximusformoso/Desktop/CSE3150/Course_Project/parser.h), [parser.cpp](/Users/maximusformoso/Desktop/CSE3150/Course_Project/parser.cpp): CAIDA topology parser
- [announcement_parser.h](/Users/maximusformoso/Desktop/CSE3150/Course_Project/announcement_parser.h), [announcement_parser.cpp](/Users/maximusformoso/Desktop/CSE3150/Course_Project/announcement_parser.cpp): announcement CSV parser
- [rov_parser.h](/Users/maximusformoso/Desktop/CSE3150/Course_Project/rov_parser.h), [rov_parser.cpp](/Users/maximusformoso/Desktop/CSE3150/Course_Project/rov_parser.cpp): ROV ASN parser
- [output.h](/Users/maximusformoso/Desktop/CSE3150/Course_Project/output.h), [output.cpp](/Users/maximusformoso/Desktop/CSE3150/Course_Project/output.cpp): output formatting and CSV writing
- [announcement.h](/Users/maximusformoso/Desktop/CSE3150/Course_Project/announcement.h): announcement structure
- [policy.h](/Users/maximusformoso/Desktop/CSE3150/Course_Project/policy.h): policy interface

## Design Decisions

- Each AS stores three adjacency lists for each type: providers, customers, and peers.
- Each AS owns a `Policy` object backed by a `BGP` implementation.
- The `BGP` object stores:
  - a local RIB mapping `prefix -> best announcement`
  - a received queue mapping `prefix -> list of candidate announcements`
- Announcements are propagated in phases rather than arbitrary repeated flooding because this is a more controlled method and follows the project specification.
- ROV is implemented as policy-time filtering so if an AS is marked as deploying ROV invalid announcements are dropped immediately when received.
- CSV parsers trim whitespace so they can handle normal files and different line endings cleanly.

## Build and Run

Compile the simulator:

```bash
g++ -std=c++20 -O3 -Isrc Include/main.cpp Include/graph.cpp Include/parser.cpp Include/bgp.cpp Include/announcement_parser.cpp Include/rov_parser.cpp Include/output.cpp Include/simulator.cpp -o simulator
```

Run the simulator:

```bash
./simulator
```

Optional arguments:

```bash
./simulator <caida_file> <rov_file> <announcements_file> <output_csv>
```

Expected output:

```text
Wrote ribs.csv
```

This produces:

- `ribs.csv`

## Web App

The repository also includes a static frontend in [web](/Users/maximusformoso/Desktop/Course_Project/web) that is designed for Cloudflare Pages. It is intended to load the simulator through WebAssembly so users can:

- upload an announcements CSV
- enter a target ASN
- inspect the AS paths learned at that ASN
- download the full generated `ribs.csv`

To build the browser bundle after installing Emscripten:

```bash
./web/build-wasm.sh
```

Then serve the `web` folder locally or deploy it directly through Cloudflare Pages.

## Default Input Files

The current `main.cpp` is configured to use:

- `bench/subprefix/CAIDAASGraphCollector_2025.10.16.txt`
- `bench/subprefix/rov_asns.csv`
- `bench/subprefix/anns.csv`

and outputs:

- `ribs.csv`

## Testing

Optional test for each file:

```bash
g++ -std=c++20 test_rov_parser.cpp rov_parser.cpp -o test_rov_parser
./test_rov_parser
```

```bash
g++ -std=c++20 test_bgp.cpp bgp.cpp -o test_bgp
./test_bgp
```

```bash
g++ -std=c++20 test_graph_propagation.cpp graph.cpp bgp.cpp -o test_graph_propagation
./test_graph_propagation
```

### Bench Comparison

After running `./simulator`, compare the generated file to the expected bench file:

```bash
diff <(tail -n +2 ribs.csv | tr -d '\r' | sort | uniq) <(tail -n +2 bench/subprefix/ribs.csv | tr -d '\r' | sort | uniq)
```

If the output is empty then the generated file matches the expected result.

## Notes

- The program checks for provider cycles and exits early if one is detected.
- The implementation supports both valid and ROV-invalid announcements.
- Output formatting matches the provided bench data, including single-element AS-path formatting such as `"(27,)"`.
# Course_Project
