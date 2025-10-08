#!/bin/bash

echo "Building sequential benchmark..."
mkdir -p out
g++ -std=c++11 -Iinclude \
    src/benchmark_sequential.cpp \
    src/branch_and_bound.cpp \
    src/knapsack_utils.cpp \
    src/output_display.cpp \
    src/parser/parser.cpp \
    -o out/benchmark_seq

if [ $? -eq 0 ]; then
    echo "✓ Build successful!"
    echo ""
    echo "Run with: ./out/benchmark_seq"
else
    echo "✗ Build failed!"
    exit 1
fi
