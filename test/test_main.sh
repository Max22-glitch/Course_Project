#!/bin/bash

g++ -std=c++20 -O3 main.cpp graph.cpp parser.cpp bgp.cpp announcement_parser.cpp rov_parser.cpp output.cpp -o simulator

if [ $? -ne 0 ]; then
    echo "Compilation failed"
    exit 1
fi

./simulator

if [ $? -ne 0 ]; then
    echo "Program run failed"
    exit 1
fi

if [ ! -f ribs.csv ]; then
    echo "ribs.csv was not created"
    exit 1
fi

echo "Full program test passed"
