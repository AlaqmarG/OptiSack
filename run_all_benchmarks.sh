#!/bin/bash

echo "======================================================================"
echo "Running Sequential Benchmarks on All Datasets"
echo "======================================================================"
echo ""

# Array of datasets: "filename:capacity:description"
datasets=(
    "baseline.txt:1466.58:Baseline (15 items, quick test)"
    "test_38_items.txt:3464.50:Main benchmark (38 items, ~40s)"
    "slow.txt:5000.0:Original slow dataset (200 items)"
)

for dataset in "${datasets[@]}"; do
    IFS=':' read -r file cap desc <<< "$dataset"
    
    echo "======================================================================"
    echo "Testing: $desc"
    echo "File: data/$file"
    echo "======================================================================"
    
    # Create test_config.h for this dataset
    cat > test_config.h << EOF
#ifndef TEST_CONFIG_H
#define TEST_CONFIG_H
#define TEST_FILE "data/$file"
#define CAPACITY ${cap}f
#endif
EOF
    
    # Compile
    g++ -std=c++11 benchmark_sequential.cpp branch_and_bound.cpp \
        knapsack_utils.cpp output_display.cpp parser/parser.cpp \
        -o benchmark_seq 2>/dev/null
    
    # Run and show only results
    ./benchmark_seq 2>&1 | grep -A 20 "^====.*RESULTS"
    
    echo ""
    echo ""
done

echo "======================================================================"
echo "Benchmark Complete!"
echo "======================================================================"
