#!/bin/bash

# Consolidated benchmark script
# Usage: ./benchmark.sh [implementations...]
# Examples:
#   ./benchmark.sh sequential          # Run sequential benchmarks
#   ./benchmark.sh openmp             # Run OpenMP benchmarks
#   ./benchmark.sh sequential openmp  # Run both

if [ $# -eq 0 ]; then
    echo "Usage: $0 [implementations...]"
    echo "Available implementations: sequential, openmp"
    exit 1
fi

# Array of datasets: "filename:description"
datasets=(
    "benchmark_fast_85items.txt:Fast (85 items)"
    "benchmark_medium_100items.txt:Medium (100 items)"
    "benchmark_medium_hard_112items.txt:Medium-Hard (112 items)"
    "benchmark_very_hard_110items.txt:Very Hard (110 items)"
    "benchmark_extreme_121items.txt:Extreme (121 items)"
    "benchmark_ultimate_121items.txt:Ultimate (121 items)"
)

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

for impl in "$@"; do
    case $impl in
        sequential)
            echo "======================================================================"
            echo "Running Sequential Benchmarks on All Datasets"
            echo "======================================================================"
            echo ""
            
            cd "$ROOT_DIR"
            mkdir -p out/sequential results
            
            # Clear previous results and write header
            > results/sequential_benchmarks.csv
            echo "dataset,implementation,threads,iterations,total_time_sec,avg_time_sec,optimal_value" > results/sequential_benchmarks.csv
            
            for dataset in "${datasets[@]}"; do
                IFS=':' read -r file desc <<< "$dataset"
                
                echo "======================================================================"
                echo "Testing: $desc"
                echo "File: data/$file"
                echo "======================================================================"
                
                # Create test_config.h for this dataset
                cat > include/sequential/test_config.h << EOF
#ifndef TEST_CONFIG_H
#define TEST_CONFIG_H
#define TEST_FILE "data/$file"
#endif
EOF
                
                # Compile
                g++ -std=c++11 -Iinclude/common -Iinclude/sequential \
                    src/sequential/benchmark_sequential.cpp \
                    src/sequential/branch_and_bound.cpp \
                    src/common/knapsack_utils.cpp \
                    src/common/output_display.cpp \
                    src/common/parser/parser.cpp \
                    -o out/sequential/benchmark_seq 2>/dev/null
                
                # Run and show only results
                ./out/sequential/benchmark_seq 2>&1 | grep -A 20 "RESULTS"
                
                echo ""
                echo ""
            done
            ;;
        openmp)
            echo "======================================================================"
            echo "Running OpenMP Benchmarks on All Datasets"
            echo "======================================================================"
            echo ""
            
            cd "$ROOT_DIR"
            mkdir -p out/openmp results
            
            # Clear previous results and write header
            > results/parallel_benchmarks.csv
            echo "dataset,implementation,threads,iterations,total_time_sec,avg_time_sec,optimal_value" > results/parallel_benchmarks.csv
            
            # Detect number of CPU cores
            if [[ "$OSTYPE" == "darwin"* ]] || [[ "$(uname)" == "Darwin" ]]; then
                NUM_CORES=$(sysctl -n hw.ncpu 2>/dev/null || echo "8")
            else
                NUM_CORES=$(nproc 2>/dev/null || echo "8")
            fi
            
            echo "Using $NUM_CORES CPU cores for parallel benchmarks"
            echo ""
            
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
            
            for dataset in "${datasets[@]}"; do
                IFS=':' read -r file desc <<< "$dataset"
                
                echo "======================================================================"
                echo "Testing: $desc"
                echo "File: data/$file"
                echo "======================================================================"
                
                # Create test_config.h for this dataset
                cat > include/openmp/test_config.h << EOF
#ifndef TEST_CONFIG_H
#define TEST_CONFIG_H
#define TEST_FILE "data/$file"
#endif
EOF
                
                # Compile
                $COMPILER -std=c++11 -Iinclude/common -Iinclude/openmp $OPENMP_FLAGS \
                    src/openmp/benchmark_parallel.cpp \
                    src/openmp/branch_and_bound_parallel.cpp \
                    src/common/knapsack_utils.cpp \
                    src/common/output_display.cpp \
                    src/common/parser/parser.cpp \
                    -o out/openmp/benchmark_par 2>/dev/null
                
                # Run and show only results
                ./out/openmp/benchmark_par $NUM_CORES 2>&1 | grep -A 20 "RESULTS"
                
                echo ""
                echo ""
            done
            ;;
        *)
            echo "Unknown implementation: $impl"
            echo "Available: sequential, openmp"
            exit 1
            ;;
    esac
done

echo "======================================================================"
echo "Benchmark Complete!"
echo "======================================================================"