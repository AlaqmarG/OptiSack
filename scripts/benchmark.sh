#!/bin/bash

# Consolidated benchmark script
# Usage: ./benchmark.sh [implementations...]
# Examples:
#   ./benchmark.sh sequential          # Run sequential benchmarks
#   ./benchmark.sh openmp             # Run OpenMP benchmarks
#   ./benchmark.sh sequential openmp  # Run both

if [ $# -eq 0 ]; then
    echo "Usage: $0 [implementations...]"
    echo "Available implementations: sequential, openmp, openmpi"
    exit 1
fi

# Array of datasets: "filename:description"
datasets=(
    "85.txt:Fast (85 items)"
    "100.txt:Medium (100 items)"
    "112.txt:Medium-Hard (112 items)"
    "110.txt:Very Hard (110 items)"
    "121.txt:Extreme (121 items)"
    "130_subset_sum.txt:Subset-sum hard (130 items)"
    "140_subset_sum.txt:Subset-sum very hard (140 items)"
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
            echo "dataset,implementation,threads,iterations,total_time_sec,avg_time_sec,nodes_explored,nodes_pruned,optimal_value" > results/sequential_benchmarks.csv
            
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
            > results/openmp_benchmarks.csv
            echo "dataset,implementation,threads,iterations,total_time_sec,avg_time_sec,nodes_explored,nodes_pruned,optimal_value" > results/openmp_benchmarks.csv
            
            # Detect number of (logical) CPU cores
            if [[ "$OSTYPE" == "darwin"* ]] || [[ "$(uname)" == "Darwin" ]]; then
                MAX_THREADS=$(sysctl -n hw.ncpu 2>/dev/null || echo "8")
            else
                MAX_THREADS=$(nproc 2>/dev/null || echo "8")
            fi

            # Build list of thread counts: 2,4,...,MAX_THREADS (and include MAX_THREADS if it's odd)
            THREAD_COUNTS=()
            for ((t=2; t<=MAX_THREADS; t+=2)); do
                THREAD_COUNTS+=("$t")
            done
            if (( MAX_THREADS % 2 == 1 && MAX_THREADS > 1 )); then
                THREAD_COUNTS+=("$MAX_THREADS")
            fi

            echo "Detected max threads: $MAX_THREADS"
            echo -n "Running OpenMP benchmarks with thread counts:"
            for t in "${THREAD_COUNTS[@]}"; do
                echo -n " $t"
            done
            echo ""
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

                # Run for each requested thread count and show only results
                for threads in "${THREAD_COUNTS[@]}"; do
                    echo ">>> Running with $threads OpenMP threads"
                    ./out/openmp/benchmark_par "$threads" 2>&1 | grep -A 20 "RESULTS"
                    echo ""
                done

                echo ""
                echo ""
            done
            ;;
        openmpi)
            echo "======================================================================"
            echo "Running OpenMPI Benchmarks on All Datasets"
            echo "======================================================================"
            echo ""

            cd "$ROOT_DIR"
            mkdir -p out/openmpi results

            > results/openmpi_benchmarks.csv
            echo "dataset,implementation,processes,iterations,total_time_sec,avg_time_sec,nodes_explored,nodes_pruned,optimal_value" > results/openmpi_benchmarks.csv

            if [[ "$OSTYPE" == "darwin"* ]] || [[ "$(uname)" == "Darwin" ]]; then
                MAX_PROCS=$(sysctl -n hw.ncpu 2>/dev/null || echo "8")
            else
                MAX_PROCS=$(nproc 2>/dev/null || echo "8")
            fi

            # Build list of process counts: 2,4,...,MAX_PROCS (and include MAX_PROCS if it's odd)
            PROC_COUNTS=()
            for ((p=2; p<=MAX_PROCS; p+=2)); do
                PROC_COUNTS+=("$p")
            done
            if (( MAX_PROCS % 2 == 1 && MAX_PROCS > 1 )); then
                PROC_COUNTS+=("$MAX_PROCS")
            fi

            if command -v mpic++ &> /dev/null; then
                MPI_COMPILER="mpic++"
            elif command -v mpicc &> /dev/null; then
                MPI_COMPILER="mpicc"
            else
                echo "Error: mpic++ (MPI C++ compiler) not found"
                exit 1
            fi

            echo "Detected max MPI processes: $MAX_PROCS"
            echo -n "Running OpenMPI benchmarks with process counts:"
            for p in "${PROC_COUNTS[@]}"; do
                echo -n " $p"
            done
            echo ""
            echo ""

            for dataset in "${datasets[@]}"; do
                IFS=':' read -r file desc <<< "$dataset"

                echo "======================================================================"
                echo "Testing: $desc"
                echo "File: data/$file"
                echo "======================================================================"

                cat > include/openmpi/test_config.h << EOF
#ifndef TEST_CONFIG_H
#define TEST_CONFIG_H
#define TEST_FILE "data/$file"
#endif
EOF

                # Detect OpenMP flags
                if [[ "$OSTYPE" == "darwin"* ]] || [[ "$(uname)" == "Darwin" ]]; then
                    LIBOMP_PREFIX=$(brew --prefix libomp 2>/dev/null || echo "/opt/homebrew/opt/libomp")
                    OPENMP_FLAGS="-Xpreprocessor -fopenmp -I${LIBOMP_PREFIX}/include -L${LIBOMP_PREFIX}/lib -lomp"
                else
                    OPENMP_FLAGS="-fopenmp"
                fi

                $MPI_COMPILER -std=c++11 -Iinclude/common -Iinclude/openmpi \
                    src/openmpi/benchmark_mpi.cpp \
                    src/openmpi/branch_and_bound_mpi.cpp \
                    src/common/knapsack_utils.cpp \
                    src/common/output_display.cpp \
                    src/common/parser/parser.cpp \
                    -o out/openmpi/benchmark_mpi 2>/dev/null

                # Run for each requested process count and show only results
                for procs in "${PROC_COUNTS[@]}"; do
                    echo ">>> Running with $procs MPI processes"
                    mpirun -np "$procs" ./out/openmpi/benchmark_mpi 2>&1 | grep -A 20 "RESULTS"
                    echo ""
                done

                echo ""
                echo ""
            done
            ;;
        *)
            echo "Unknown implementation: $impl"
            echo "Available: sequential, openmp, openmpi"
            exit 1
            ;;
    esac
done

echo "======================================================================"
echo "Benchmark Complete!"
echo "======================================================================"