#!/bin/bash

# Script to run parallel benchmark with different thread counts
# Usage: ./run_benchmark.sh [dataset_name] [thread_counts...]
# Example: ./run_benchmark.sh fast 1 2 4 8

if [ ! -f "out/benchmark_parallel" ]; then
    echo "Error: benchmark_parallel not found. Run build_benchmark.sh first."
    exit 1
fi

# Default dataset
DATASET="${1:-fast}"
THREAD_COUNTS="${@:2}"

# Default thread counts if not provided
if [ -z "$THREAD_COUNTS" ]; then
    THREAD_COUNTS="1 2 4 8"
fi

# Update test config to use specified dataset
CONFIG_FILE="include/test_config.h"

# Handle special case for "huge" dataset
if [ "$DATASET" = "huge" ]; then
    DATASET_FILE="benchmark_huge_30000items.txt"
elif [ -f "../sequential/data/benchmark_${DATASET}_*.txt" ]; then
    # Find the exact filename
    DATASET_FILE=$(ls ../sequential/data/benchmark_${DATASET}_*.txt 2>/dev/null | head -n 1)
    DATASET_FILE=$(basename "$DATASET_FILE")
else
    echo "Warning: Dataset file not found for '$DATASET', using default from test_config.h"
    DATASET_FILE=""
fi

if [ -n "$DATASET_FILE" ]; then
    echo "#ifndef TEST_CONFIG_H" > "$CONFIG_FILE"
    echo "#define TEST_CONFIG_H" >> "$CONFIG_FILE"
    echo "#define TEST_FILE \"data/$DATASET_FILE\"" >> "$CONFIG_FILE"
    echo "#endif" >> "$CONFIG_FILE"
    echo "Using dataset: $DATASET_FILE"
fi

echo ""
echo "Running parallel benchmarks with thread counts: $THREAD_COUNTS"
echo "================================================================"
echo ""

for threads in $THREAD_COUNTS; do
    echo "--- Running with $threads thread(s) ---"
    ./out/benchmark_parallel $threads
    echo ""
done

echo "================================================================"
echo "Benchmark complete!"

