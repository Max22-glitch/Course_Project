#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
OUT_DIR="$ROOT_DIR/web/assets/wasm"
CACHE_DIR="$ROOT_DIR/web/.emscripten-cache"

mkdir -p "$OUT_DIR"
mkdir -p "$CACHE_DIR"

export EM_CACHE="$CACHE_DIR"

em++ \
  -std=c++20 \
  -O3 \
  -Isrc \
  Include/bgp.cpp \
  Include/graph.cpp \
  Include/parser.cpp \
  Include/announcement_parser.cpp \
  Include/rov_parser.cpp \
  Include/output.cpp \
  Include/simulator.cpp \
  Include/wasm_bridge.cpp \
  -s WASM=1 \
  -s MODULARIZE=1 \
  -s EXPORT_NAME=createSimulatorModule \
  -s EXPORT_ES6=1 \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s INITIAL_MEMORY=268435456 \
  -s MAXIMUM_MEMORY=1073741824 \
  -s STACK_SIZE=5242880 \
  -s ENVIRONMENT=web \
  -s EXPORTED_FUNCTIONS='["_run_simulation_json","_malloc","_free"]' \
  -s EXPORTED_RUNTIME_METHODS='["ccall","stringToUTF8","UTF8ToString","lengthBytesUTF8"]' \
  --no-entry \
  -o "$OUT_DIR/bgp_simulator.js"

echo "Built WASM bundle in $OUT_DIR"
