#!/bin/bash

# Consolidated run script
# Usage: ./run.sh <dataset> <implementation> [num_threads]
# Examples:
#   ./run.sh benchmark_ultimate_121items.txt sequential
#   ./run.sh benchmark_medium_100items.txt openmp 4

if [ $# -lt 2 ]; then
    echo "Usage: $0 <dataset> <implementation> [num_threads]"
    echo "Datasets: benchmark_*.txt files in data/"
    echo "Implementations: sequential, openmp"
    exit 1
fi

DATASET="$1"
IMPL="$2"
NUM_THREADS="$3"  # Optional third parameter

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

# If no thread count specified for OpenMP, detect number of cores
if [ "$IMPL" = "openmp" ] && [ -z "$NUM_THREADS" ]; then
    if [[ "$OSTYPE" == "darwin"* ]] || [[ "$(uname)" == "Darwin" ]]; then
        NUM_THREADS=$(sysctl -n hw.ncpu 2>/dev/null || echo "8")
    else
        NUM_THREADS=$(nproc 2>/dev/null || echo "8")
    fi
    echo "Auto-detected $NUM_THREADS CPU cores for OpenMP"
fi

# Default to 4 threads if still not set (for backward compatibility)
NUM_THREADS="${NUM_THREADS:-4}"

case $IMPL in
    sequential)
        cd "$ROOT_DIR"
        mkdir -p out/sequential
        
        # Create test_config.h for this dataset
        cat > include/sequential/test_config.h << EOF
#ifndef TEST_CONFIG_H
#define TEST_CONFIG_H
#define TEST_FILE "data/$DATASET"
#endif
EOF
        
        # Compile
        g++ -std=c++11 -Iinclude/common -Iinclude/sequential \
            src/sequential/index.cpp \
            src/sequential/branch_and_bound.cpp \
            src/common/knapsack_utils.cpp \
            src/common/output_display.cpp \
            src/common/parser/parser.cpp \
            -o out/sequential/index 2>/dev/null
        
        if [ $? -eq 0 ]; then
            echo "✓ Build successful!"
            echo ""
            echo "Running sequential version..."
            echo ""
            ./out/sequential/index
        else
            echo "✗ Build failed!"
            exit 1
        fi
        ;;
    openmp)
        cd "$ROOT_DIR"
        mkdir -p out/openmp
        
        # Detect compiler and set OpenMP flags
        if command -v g++ &> /dev/null; then
            if g++ --version 2>&1 | grep -q "Free Software Foundation" || g++ --version 2>&1 | grep -q "GCC"; then
                COMPILER="g++"
                OPENMP_FLAGS="-fopenmp"
            else
                COMPILER="g++"
                if [[ "$OSTYPE" == "darwin"* ]] || [[ "$(uname)" == "Darwin" ]]; then
                    LIBOMP_PREFIX=$(brew --prefix libomp 2>/dev/null || echo "/opt/homebrew/opt/libomp")
                    OPENMP_FLAGS="-Xpreprocessor -fopenmp -I${LIBOMP_PREFIX}/include -L${LIBOMP_PREFIX}/lib -lomp"
                else
                    OPENMP_FLAGS="-fopenmp"
                fi
            fi
        elif command -v clang++ &> /dev/null; then
            COMPILER="clang++"
            if [[ "$OSTYPE" == "darwin"* ]] || [[ "$(uname)" == "Darwin" ]]; then
                LIBOMP_PREFIX=$(brew --prefix libomp 2>/dev/null || echo "/opt/homebrew/opt/libomp")
                OPENMP_FLAGS="-Xpreprocessor -fopenmp -I${LIBOMP_PREFIX}/include -L${LIBOMP_PREFIX}/lib -lomp"
            else
                OPENMP_FLAGS="-fopenmp"
            fi
        else
            echo "Error: No suitable compiler found"
            exit 1
        fi
        
        # Create test_config.h for this dataset
        cat > include/openmp/test_config.h << EOF
#ifndef TEST_CONFIG_H
#define TEST_CONFIG_H
#define TEST_FILE "data/$DATASET"
#endif
EOF
        
        # Compile
        $COMPILER -std=c++11 -Iinclude/common -Iinclude/openmp $OPENMP_FLAGS \
            src/openmp/index.cpp \
            src/openmp/branch_and_bound_parallel.cpp \
            src/common/knapsack_utils.cpp \
            src/common/output_display.cpp \
            src/common/parser/parser.cpp \
            -o out/openmp/index 2>/dev/null
        
        if [ $? -eq 0 ]; then
            echo "✓ Build successful!"
            echo ""
            echo "Running OpenMP version with $NUM_THREADS threads..."
            echo ""
            ./out/openmp/index $NUM_THREADS
        else
            echo "✗ Build failed!"
            exit 1
        fi
        ;;
    *)
        echo "Unknown implementation: $IMPL"
        echo "Available: sequential, openmp"
        exit 1
        ;;
esac