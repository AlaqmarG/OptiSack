# Parallel Branch and Bound Knapsack Solver (OpenMP)

A parallel implementation of the 0/1 Knapsack Problem using Branch and Bound with OpenMP. Multiple worker threads explore different branches of the search tree concurrently, using a shared thread-safe priority queue for dynamic load balancing.

## Overview

This program solves the 0/1 Knapsack Problem optimally using a parallel branch and bound algorithm. The implementation follows Algorithm 2 from the project guidelines, using:

- **Shared Work Pool**: Thread-safe priority queue accessible by all worker threads
- **Multiple Worker Threads**: Each thread extracts nodes, calculates bounds, generates children, and adds promising nodes back
- **Global Best Solution**: Shared variable tracking the current best solution value with proper synchronization
- **Termination Detection**: Coordinated termination when queue is empty and all threads are idle

## Project Structure

```
openmp/
├── src/                 # Source files
│   ├── index.cpp
│   ├── benchmark_parallel.cpp
│   ├── branch_and_bound_parallel.cpp
│   ├── knapsack_utils.cpp
│   ├── output_display.cpp
│   └── parser/
│       └── parser.cpp
├── include/             # Header files
│   ├── branch_and_bound_parallel.h
│   ├── knapsack_utils.h
│   ├── output_display.h
│   ├── tree_node.h
│   ├── test_config.h
│   └── parser/
│       └── parser.h
├── scripts/             # Build and run scripts
│   ├── build_and_run.sh
│   ├── build_benchmark.sh
│   └── run_benchmark.sh
├── data/                # Symlink to sequential/data
└── README.md
```

## Building and Running

### Prerequisites

- C++11 compatible compiler (g++ or clang++)
- OpenMP support (usually included with g++, may need `libomp` for clang on macOS)
- Unix-like OS (Linux, macOS)

### Quick Start

Run the main program with default 4 threads:

```bash
cd openmp
./scripts/build_and_run.sh
```

Run with a specific number of threads:

```bash
./scripts/build_and_run.sh 8
```

### Manual Build

```bash
mkdir -p out
g++ -std=c++11 -I../sequential/include -Iinclude -fopenmp \
    src/index.cpp \
    src/branch_and_bound_parallel.cpp \
    src/knapsack_utils.cpp \
    src/output_display.cpp \
    src/parser/parser.cpp \
    -o out/index
./out/index [num_threads]
```

**Note**: On macOS with Clang, you may need:
```bash
clang++ -std=c++11 -I../sequential/include -Iinclude \
    -Xpreprocessor -fopenmp -lomp \
    src/index.cpp ... -o out/index
```

### Run Benchmarks

```bash
# Build benchmark executable first
./scripts/build_benchmark.sh

# Run single dataset with multiple thread counts
./scripts/run_benchmark.sh fast
./scripts/run_benchmark.sh fast 1 2 4 8 16

# Run ALL datasets with multiple thread counts (comprehensive)
./scripts/run_all_benchmarks.sh
./scripts/run_all_benchmarks.sh 1 2 4 8 16

# Run comprehensive benchmarks with CSV output
./scripts/run_final_benchmarks.sh
./scripts/run_final_benchmarks.sh 1 2 4 8

# Run benchmark directly
./out/benchmark_parallel 4
```

## Configuration

Configure which dataset to use by editing `include/test_config.h`:

```cpp
#define TEST_FILE "../sequential/data/benchmark_medium_100items.txt"
```

Available datasets (in `../sequential/data/`):
- `benchmark_fast_85items.txt`
- `benchmark_medium_100items.txt`
- `benchmark_medium_hard_112items.txt`
- `benchmark_very_hard_110items.txt`
- `benchmark_extreme_121items.txt`
- `benchmark_ultimate_121items.txt`
- `benchmark_huge_30000items.txt`

## Parallelization Details

### Synchronization Mechanisms

1. **Queue Access**: Protected by `omp_lock_t` - threads lock before pop/push operations
2. **Global Best Value**: Protected by critical sections for thread-safe reads/writes
3. **Termination Detection**: Uses idle thread counter with double-checking to avoid race conditions

### Algorithm Flow (per worker thread)

1. Lock queue
2. Check if empty → if yes, mark as idle and check termination condition
3. Pop highest-bound node
4. Unlock queue
5. Check bound against global best (critical section read)
6. If promising, expand node (generate children)
7. Calculate bounds for children
8. Update global best if found better solution (lock-protected)
9. Lock queue, add promising children, unlock queue
10. Repeat until termination

### Performance Considerations

- **Lock Contention**: The shared priority queue can become a bottleneck with many threads
- **Load Balancing**: Dynamic work stealing helps balance workload across threads
- **Termination Overhead**: Double-checking termination adds minimal overhead
- **Scalability**: Performance improvement depends on problem size and thread count

## Comparison with Sequential Version

The parallel version maintains the same correctness guarantees as the sequential version:
- Same optimal solution
- Same pruning strategy
- Same bound calculations

Performance improvements depend on:
- Number of threads
- Problem size (larger problems benefit more)
- Hardware (number of CPU cores)

Expected speedup is typically sub-linear due to:
- Lock contention on shared queue
- Load imbalance between branches
- Synchronization overhead

## Requirements

- C++11 or later
- OpenMP support
- g++ or clang++ compiler
- Unix-like OS (Linux, macOS)
