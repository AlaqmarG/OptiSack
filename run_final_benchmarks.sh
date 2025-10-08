#!/bin/bash
# Run full 50-iteration benchmarks on selected datasets for manuscript

echo "======================================================================"
echo "Running Full Benchmarks (50 iterations each)"
echo "======================================================================"
echo ""

# Create results directory
mkdir -p results
RESULTS_FILE="results/final_benchmarks.csv"

# Write CSV header
echo "dataset,items,capacity,avg_time_s,total_time_s,optimal_value" > $RESULTS_FILE

# Datasets to benchmark
datasets=(
    "data/benchmark_fast_85items.txt:Fast Baseline"
    "data/benchmark_medium_100items.txt:Medium Difficulty"
    "data/benchmark_hard_112items.txt:Hard"
    "data/benchmark_vhard_110items.txt:Very Hard"
    "data/benchmark_extreme_121items.txt:Extreme"
)

for dataset_info in "${datasets[@]}"; do
    IFS=':' read -r dataset_file description <<< "$dataset_info"
    
    echo "======================================================================"
    echo "Benchmarking: $description"
    echo "File: $dataset_file"
    echo "======================================================================"
    
    # Update test_config.h to point to this dataset
    cat > test_config.h << EOF
#ifndef TEST_CONFIG_H
#define TEST_CONFIG_H
#define TEST_FILE "$dataset_file"
#endif
EOF
    
    # Compile benchmark
    g++ -std=c++11 benchmark_sequential.cpp branch_and_bound.cpp \
        knapsack_utils.cpp output_display.cpp parser/parser.cpp \
        -o benchmark_seq 2>/dev/null
    
    if [ $? -ne 0 ]; then
        echo "Build failed!"
        continue
    fi
    
    # Run benchmark
    ./benchmark_seq > temp_benchmark_output.txt 2>&1
    
    # Extract results
    avg_time=$(grep "Average time per run:" temp_benchmark_output.txt | grep -o "[0-9.]*" | head -1)
    total_time=$(grep "Total time.*runs" temp_benchmark_output.txt | grep -o "[0-9.]*" | head -1)
    optimal=$(grep "Optimal value:" temp_benchmark_output.txt | grep -o "[0-9.]*" | head -1)
    items=$(grep "Loaded.*items" temp_benchmark_output.txt | grep -o "[0-9]*" | head -1)
    
    # Show summary
    cat temp_benchmark_output.txt | grep -A 10 "^====.*RESULTS"
    
    echo ""
    echo ""
done

rm -f temp_benchmark_output.txt

echo "======================================================================"
echo "All Benchmarks Complete!"
echo "======================================================================"
echo ""
echo "Results summary:"
cat $RESULTS_FILE | column -t -s ','

