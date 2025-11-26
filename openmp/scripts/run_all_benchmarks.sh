#!/bin/bash

# Run parallel benchmarks on all datasets with multiple thread counts
# Usage: ./run_all_benchmarks.sh [thread_counts...]
# Example: ./run_all_benchmarks.sh 1 2 4 8

if [ ! -f "out/benchmark_parallel" ]; then
    echo "Error: benchmark_parallel not found. Run build_benchmark.sh first."
    exit 1
fi

# Default thread counts
THREAD_COUNTS="${@:-1 2 4 8}"

echo "======================================================================"
echo "Running Parallel Benchmarks on All Datasets"
echo "Thread counts: $THREAD_COUNTS"
echo "======================================================================"
echo ""

# Array of datasets: "filename_prefix:description"
datasets=(
    "fast:Fast (85 items)"
    "medium:Medium (100 items)"
    "medium_hard:Medium-Hard (112 items)"
    "very_hard:Very Hard (110 items)"
    "extreme:Extreme (121 items)"
    "ultimate:Ultimate (121 items)"
)

for dataset_info in "${datasets[@]}"; do
    IFS=':' read -r prefix desc <<< "$dataset_info"
    
    echo "======================================================================"
    echo "Testing: $desc"
    echo "======================================================================"
    
    # Find the exact dataset file
    DATASET_FILE=$(ls ../sequential/data/benchmark_${prefix}_*.txt 2>/dev/null | head -n 1)
    if [ -z "$DATASET_FILE" ]; then
        echo "Warning: Dataset file not found for prefix '$prefix', skipping..."
        echo ""
        continue
    fi
    
    DATASET_FILE=$(basename "$DATASET_FILE")
    
    # Update test_config.h for this dataset
    cat > include/test_config.h << EOF
#ifndef TEST_CONFIG_H
#define TEST_CONFIG_H
#define TEST_FILE "data/$DATASET_FILE"
#endif
EOF
    
    echo "Dataset: $DATASET_FILE"
    echo ""
    
    # Run benchmark for each thread count
    for threads in $THREAD_COUNTS; do
        echo "--- $desc: $threads thread(s) ---"
        ./out/benchmark_parallel $threads 2>&1 | grep -A 10 "^====.*RESULTS" || ./out/benchmark_parallel $threads 2>&1 | tail -10
        echo ""
    done
    
    echo ""
    echo ""
done

echo "======================================================================"
echo "Benchmark Complete!"
echo "======================================================================"


