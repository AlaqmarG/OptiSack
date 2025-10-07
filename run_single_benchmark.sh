#!/bin/bash

# Helper script to run benchmark on a specific dataset
# Usage: ./run_single_benchmark.sh [dataset]
# Example: ./run_single_benchmark.sh fast

if [ $# -eq 0 ]; then
    echo "Usage: ./run_single_benchmark.sh [dataset]"
    echo ""
    echo "Available datasets:"
    echo "  baseline    - 15 items (quick correctness test)"
    echo "  fast        - 38 items (good runtime ~40s for 50 iterations)"
    echo "  slow        - 200 items (original test file)"
    echo ""
    echo "Example: ./run_single_benchmark.sh fast"
    exit 1
fi

DATASET=$1

# Set file and capacity based on input
case $DATASET in
    baseline)
        FILE="baseline.txt"
        CAP="1466.58"
        ;;
    fast)
        FILE="test_38_items.txt"
        CAP="3464.50"
        ;;
    slow)
        FILE="slow.txt"
        CAP="5000.0"
        ;;
    *)
        echo "Unknown dataset: $DATASET"
        echo "Run without arguments to see available options"
        exit 1
        ;;
esac

echo "======================================================================"
echo "Running Benchmark: $FILE"
echo "======================================================================"

# Create config
cat > test_config.h << EOF
#ifndef TEST_CONFIG_H
#define TEST_CONFIG_H
#define TEST_FILE "data/$FILE"
#define CAPACITY ${CAP}f
#endif
EOF

# Build
echo "Building..."
./build_benchmark.sh > /dev/null 2>&1

if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

# Run
echo ""
./benchmark_seq
