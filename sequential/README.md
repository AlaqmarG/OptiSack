# Branch and Bound Knapsack Solver

A sequential implementation of the 0/1 Knapsack Problem using Branch and Bound with best-first search strategy.

## Overview

This program solves the 0/1 Knapsack Problem optimally using a priority queue-based Branch and Bound algorithm. Items are sorted by value-to-weight ratio, and the search tree is pruned using upper bound calculations to eliminate unpromising branches.

## Project Structure

```
ps2/
├── src/                 # Source files
│   ├── index.cpp
│   ├── benchmark_sequential.cpp
│   ├── branch_and_bound.cpp
│   ├── knapsack_utils.cpp
│   ├── output_display.cpp
│   ├── generate_dataset.cpp
│   └── parser/
│       └── parser.cpp
├── include/             # Header files
│   ├── branch_and_bound.h
│   ├── knapsack_utils.h
│   ├── output_display.h
│   ├── tree_node.h
│   ├── test_config.h
│   └── parser/
│       └── parser.h
├── scripts/             # Build and run scripts
│   ├── build_and_run.sh
│   ├── build_benchmark.sh
│   ├── run_single_benchmark.sh
│   ├── run_all_benchmarks.sh
│   └── run_final_benchmarks.sh
├── data/                # Test datasets
├── out/                 # Build outputs (generated)
└── README.md
```

## Building and Running

### Quick Start

Run the main program with the configured dataset:

```bash
./scripts/build_and_run.sh
```

Configure which dataset to use by editing `include/test_config.h`:

```cpp
#define TEST_FILE "data/benchmark_medium_100items.txt"
```

### Manual Build

```bash
mkdir -p out
g++ -std=c++11 -Iinclude \
    src/index.cpp \
    src/branch_and_bound.cpp \
    src/knapsack_utils.cpp \
    src/output_display.cpp \
    src/parser/parser.cpp \
    -o out/index
./out/index
```

### Run Benchmarks

```bash
# Build benchmark executable
./scripts/build_benchmark.sh

# Run single dataset (choose: fast, medium, medium-hard, very-hard, extreme, ultimate)
./scripts/run_single_benchmark.sh fast

# Run all datasets (50 iterations each)
./scripts/run_all_benchmarks.sh

# Run final benchmarks with CSV output
./scripts/run_final_benchmarks.sh
```

## Input Format

Data files use the following format:

```
<item_count>
<id_0> <value_0> <weight_0>
<id_1> <value_1> <weight_1>
...
<capacity>
```

Example:

```
3
0 100.0 50.0
1 80.0 40.0
2 60.0 30.0
200.0
```

## Available Datasets

| File                                 | Items | Difficulty  |
| ------------------------------------ | ----- | ----------- |
| `benchmark_fast_85items.txt`         | 85    | Easy        |
| `benchmark_medium_100items.txt`      | 100   | Medium      |
| `benchmark_medium_hard_112items.txt` | 112   | Medium-Hard |
| `benchmark_very_hard_110items.txt`   | 110   | Very Hard   |
| `benchmark_extreme_121items.txt`     | 121   | Extreme     |
| `benchmark_ultimate_121items.txt`    | 121   | Ultimate    |
| `benchmark_huge_30000items.txt`      | 30000 | Massive     |

## Requirements

-   C++11 or later
-   g++ or clang++
-   Unix-like OS (Linux, macOS)
