#!/bin/bash
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SEQUENTIAL_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)/sequential"

cd "$SEQUENTIAL_DIR"

mkdir -p out
g++ -std=c++11 -I../include/common -I../include/sequential \
    ../src/sequential/index.cpp \
    ../src/sequential/branch_and_bound.cpp \
    ../src/common/knapsack_utils.cpp \
    ../src/common/output_display.cpp \
    ../src/common/parser/parser.cpp \
    -o out/index
time ./out/index
rm -rf out
