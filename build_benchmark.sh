#!/bin/bash

echo "Building sequential benchmark..."
g++ -std=c++11 \
    benchmark_sequential.cpp \
    branch_and_bound.cpp \
    knapsack_utils.cpp \
    output_display.cpp \
    parser/parser.cpp \
    -o out/benchmark_seq

if [ $? -eq 0 ]; then
    echo "✓ Build successful!"
    echo ""
    echo "Run with: ./benchmark_seq"
else
    echo "✗ Build failed!"
    exit 1
fi

