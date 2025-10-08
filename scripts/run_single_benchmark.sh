#!/bin/bash

# Helper script to run benchmark on a specific dataset
# Usage: ./run_single_benchmark.sh [dataset]
# Example: ./run_single_benchmark.sh fast

if [ $# -eq 0 ]; then
    echo "Usage: ./scripts/run_single_benchmark.sh [dataset]"
    echo ""
    echo "Available datasets:"
    echo "  fast         - 85 items (quick test)"
    echo "  medium       - 100 items"
    echo "  medium-hard  - 112 items"
    echo "  very-hard    - 110 items"
    echo "  extreme      - 121 items"
    echo "  ultimate     - 121 items (hardest)"
    echo ""
    echo "Example: ./scripts/run_single_benchmark.sh fast"
    exit 1
fi

DATASET=$1

# Set file based on input
case $DATASET in
    fast)
        FILE="benchmark_fast_85items.txt"
        ;;
    medium)
        FILE="benchmark_medium_100items.txt"
        ;;
    medium-hard)
        FILE="benchmark_medium_hard_112items.txt"
        ;;
    very-hard)
        FILE="benchmark_very_hard_110items.txt"
        ;;
    extreme)
        FILE="benchmark_extreme_121items.txt"
        ;;
    ultimate)
        FILE="benchmark_ultimate_121items.txt"
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
cat > include/test_config.h << EOF
#ifndef TEST_CONFIG_H
#define TEST_CONFIG_H
#define TEST_FILE "data/$FILE"
#endif
EOF

# Build
echo "Building..."
./scripts/build_benchmark.sh > /dev/null 2>&1

if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

# Run
echo ""
./out/benchmark_seq
