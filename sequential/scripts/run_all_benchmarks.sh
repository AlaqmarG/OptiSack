#!/bin/bash

echo "======================================================================"
echo "Running Sequential Benchmarks on All Datasets"
echo "======================================================================"
echo ""

# Array of datasets: "filename:description"
datasets=(
    "benchmark_fast_85items.txt:Fast (85 items)"
    "benchmark_medium_100items.txt:Medium (100 items)"
    "benchmark_medium_hard_112items.txt:Medium-Hard (112 items)"
    "benchmark_very_hard_110items.txt:Very Hard (110 items)"
    "benchmark_extreme_121items.txt:Extreme (121 items)"
    "benchmark_ultimate_121items.txt:Ultimate (121 items)"
)

for dataset in "${datasets[@]}"; do
    IFS=':' read -r file desc <<< "$dataset"
    
    echo "======================================================================"
    echo "Testing: $desc"
    echo "File: data/$file"
    echo "======================================================================"
    
    # Create test_config.h for this dataset
    cat > include/test_config.h << EOF
#ifndef TEST_CONFIG_H
#define TEST_CONFIG_H
#define TEST_FILE "data/$file"
#endif
EOF
    
    # Compile
    mkdir -p out
    g++ -std=c++11 -Iinclude \
        src/benchmark_sequential.cpp \
        src/branch_and_bound.cpp \
        src/knapsack_utils.cpp \
        src/output_display.cpp \
        src/parser/parser.cpp \
        -o out/benchmark_seq 2>/dev/null
    
    # Run and show only results
    ./out/benchmark_seq 2>&1 | grep -A 20 "^====.*RESULTS"
    
    echo ""
    echo ""
done

echo "======================================================================"
echo "Benchmark Complete!"
echo "======================================================================"
