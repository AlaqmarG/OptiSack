# Branch and Bound Knapsack Solver

A sequential implementation of the 0/1 Knapsack Problem using Branch and Bound with best-first search strategy.

## Overview

This program solves the 0/1 Knapsack Problem optimally using a priority queue-based Branch and Bound algorithm. Items are sorted by value-to-weight ratio, and the search tree is pruned using upper bound calculations to eliminate unpromising branches.

## Building and Running

### Quick Start

```bash
./build.sh
```

### Manual Build

```bash
mkdir -p out
g++ -std=c++11 index.cpp branch_and_bound.cpp knapsack_utils.cpp \
    output_display.cpp parser/parser.cpp -o out/index
./out/index
```

### Configure Dataset

Edit `test_config.h` to specify input file:

```cpp
#define TEST_FILE "data/benchmark_medium_100items.txt"
```

### Run Benchmarks

```bash
# Single benchmark (50 iterations)
./run_single_benchmark.sh

# All datasets
./run_all_benchmarks.sh
```

## Input Format

```
<capacity>
<item_count>
<value_1> <weight_1>
<value_2> <weight_2>
...
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
