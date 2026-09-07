# OptiSack: Parallel Branch-and-Bound Knapsack Solver

<p align="center">
  <a href="https://opensource.org/licenses/MIT"><img src="https://img.shields.io/badge/License-MIT-green.svg?style=flat-square" alt="License"></a>
  <a href="https://en.wikipedia.org/wiki/C%2B%2B11"><img src="https://img.shields.io/badge/C%2B%2B-11-00599C.svg?style=flat-square" alt="C++"></a>
  <a href="https://www.openmp.org/"><img src="https://img.shields.io/badge/OpenMP-4.5-009539.svg?style=flat-square" alt="OpenMP"></a>
  <a href="https://www.open-mpi.org/"><img src="https://img.shields.io/badge/OpenMPI-4.1-F7931E.svg?style=flat-square" alt="OpenMPI"></a>
</p>

<p align="center">
  <strong>Department of Computer Science, Brock University</strong><br>
  <strong>COSC 3P93: Parallel Computing</strong><br>
  <em>High-performance parallel implementations of the 0/1 Knapsack Problem</em>
</p>

## 📋 Abstract

This project implements and compares multiple parallel approaches to solving the 0/1 Knapsack Problem using branch-and-bound algorithms. The implementation includes sequential, shared-memory (OpenMP), and distributed-memory (OpenMPI) versions, providing a comprehensive analysis of parallel computing techniques for combinatorial optimization problems.

## ✨ Key Features

- **Multiple Parallel Paradigms**: Sequential baseline, OpenMP shared-memory, and OpenMPI distributed-memory implementations
- **Comprehensive Benchmarking**: Six diverse datasets with automated performance analysis
- **Optimal Solutions**: Guaranteed optimal results using efficient branch-and-bound pruning
- **Performance Analytics**: Detailed statistics and CSV output for comparative analysis
- **Cross-Platform Support**: Compatible with macOS and Linux systems
- **Extensive Documentation**: Complete algorithm explanations and usage examples

## 📋 Table of Contents

- [Abstract](#-abstract)
- [Key Features](#-key-features)
- [Quick Start](#-quick-start)
- [Installation](#-installation)
- [Usage](#-usage)
- [Performance Analysis](#-performance-analysis)
- [Algorithm Implementation](#-algorithm-implementation)
- [Project Structure](#-project-structure)
- [Methodology](#-methodology)
- [Results & Discussion](#-results--discussion)
- [Author](#-author)
- [References](#-references)

## 🚀 Quick Start

```bash
# Get the code
git clone https://github.com/AlaqmarG/OptiSack.git
cd OptiSack

# Try it out with OpenMP (auto-detects your CPU cores)
./scripts/run.sh benchmark_medium_100items.txt openmp

# Run the full benchmark suite
./scripts/benchmark.sh openmp openmpi
```

## 📦 Installation

### What You Need

- **C++ Compiler**: GCC 7+ or Clang 5+ (C++11 support)
- **OpenMP**: `libomp` (on macOS: `brew install libomp`)
- **OpenMPI**: `openmpi` (on macOS: `brew install openmpi`)

### Build & Run

The build scripts handle all the compilation flags automatically:

```bash
# Sequential version
./scripts/run.sh benchmark_fast_85items.txt sequential

# OpenMP version (uses all your cores)
./scripts/run.sh benchmark_medium_100items.txt openmp

# OpenMPI version (distributed across processes)
./scripts/run.sh benchmark_very_hard_110items.txt openmpi
```

## 💡 Usage

### Basic Commands

```bash
# Run a dataset with specific implementation and core count
./scripts/run.sh benchmark_medium_100items.txt openmp 8

# Available implementations:
# - sequential: Single-threaded (good baseline)
# - openmp: Shared-memory parallelism
# - openmpi: Distributed-memory parallelism
```

### Running Benchmarks

Test everything across all datasets:

```bash
# Just OpenMP
./scripts/benchmark.sh openmp

# Compare all implementations
./scripts/benchmark.sh sequential openmp openmpi

# Results go to results/*.csv
```

### Dataset Options

| Dataset | Items | Difficulty | Use Case |
|---------|-------|------------|----------|
| `benchmark_fast_85items.txt` | 85 | Easy | Quick tests |
| `benchmark_medium_100items.txt` | 100 | Medium | General use |
| `benchmark_medium_hard_112items.txt` | 112 | Hard | Performance testing |
| `benchmark_very_hard_110items.txt` | 110 | Very Hard | Stress testing |
| `benchmark_extreme_121items.txt` | 121 | Extreme | Algorithm limits |
| `benchmark_ultimate_121items.txt` | 121 | Ultimate | Max challenge |

## 📊 Performance Analysis

### Benchmark Results (Apple M2 MacBook Air - 8-core CPU)

All figures below are recomputed directly from the committed CSVs in `results/`,
as mean time per run over 10 iterations. For each dataset the best thread count
(OpenMP) or process count (OpenMPI) is reported.

| Dataset | Sequential | OpenMP (best) | Speedup | OpenMPI (best) | Speedup |
|---------|-----------:|--------------:|--------:|---------------:|--------:|
| 85 items | 121.4 ms | 81.8 ms (4 threads) | 1.48x | 286.4 ms (2 procs) | 0.42x |
| 100 items | 157.7 ms | 77.5 ms (4 threads) | 2.04x | 2550.8 ms (2 procs) | 0.06x |
| 110 items | 79.7 ms | 66.3 ms (6 threads) | 1.20x | 149.8 ms (2 procs) | 0.53x |
| 112 items | 129.1 ms | 102.1 ms (4 threads) | 1.26x | 5029.3 ms (2 procs) | 0.03x |
| 121 items | 5179.2 ms | 901.7 ms (4 threads) | 5.74x | 1725.7 ms (2 procs) | 3.00x |
| 130 (subset sum) | 2856.7 ms | 359.6 ms (4 threads) | **7.94x** | 738.8 ms (2 procs) | 3.87x |
| 140 (subset sum) | 1435.2 ms | 1863.5 ms (4 threads) | 0.77x | 679.4 ms (2 procs) | 2.11x |

> **Correction.** Earlier versions of this README reported speedups of 4794x and
> 7990x. Those numbers were wrong and are not reproducible from the data in this
> repository. The real best case is **7.94x** with OpenMP on the 130-item subset-sum
> instance. Nothing about branch-and-bound on a single 8-core laptop can produce a
> four-figure speedup, and the table above replaces the claim that it did.

### Performance Analysis

The experimental results demonstrate exceptional performance improvements through parallelization on Apple Silicon:

- **OpenMP**: Speedup grows with the search tree. It is roughly flat (1.2x-2.0x) on the small instances, where the parallel region barely amortizes its own overhead, and reaches 5.7x-7.9x on the two instances that take seconds rather than milliseconds. On 140-item subset sum it is a 0.77x *slowdown*.
- **OpenMPI**: Slower than the sequential baseline on four of seven datasets, badly so on 112 items (0.03x). Every rank redundantly explores the same 5.5M nodes on the smaller instances, so message-passing overhead is paid for no reduction in work. It only pays off on the two largest instances.
- **Optimality**: The strongest result here. All three implementations agree on the optimal value for all seven datasets, so the parallel pruning does not sacrifice correctness.
- **Honest read**: On one 8-core laptop this is a study of when parallel branch-and-bound stops being worth it, not a demonstration of large speedups.

## 🧠 Algorithm Implementation

### Branch and Bound Algorithm

The implementation employs a branch-and-bound algorithm with the following key components:

- **Bounding Strategy**: Utilizes fractional knapsack relaxation to compute tight upper bounds
- **Pruning Mechanism**: Eliminates suboptimal branches using bound comparisons
- **Search Strategy**: Implements best-first exploration using priority queues
- **Optimality Guarantee**: Ensures finding of truly optimal solutions through complete search space coverage

### Parallelization Strategies

#### OpenMP (Shared Memory Parallelism)
- **Task-based Parallelism**: Utilizes `#pragma omp task` directives for dynamic task creation
- **Synchronization Strategy**: Implements periodic synchronization every 100 nodes to minimize overhead
- **Work Distribution**: Assigns different initial decision points to each thread
- **Thread Safety**: Employs lock-based mechanisms for thread-safe global best solution sharing

#### OpenMPI (Distributed Memory Parallelism)
- **Process-based Parallelism**: Distributes work across MPI ranks
- **Global Synchronization**: Uses `MPI_Allreduce` operations for global best solution synchronization
- **Work Distribution**: Assigns different initial decision points to each MPI rank
- **Collective Operations**: Implements collective operations for comprehensive statistics aggregation

## 📁 Project Structure

```
OptiSack/
├── data/                    # Test datasets
├── include/                 # Header files
│   ├── common/             # Shared utilities
│   ├── sequential/         # Sequential headers
│   ├── openmp/            # OpenMP headers
│   └── openmpi/           # OpenMPI headers
├── src/                    # Source code
│   ├── common/            # Shared implementations
│   ├── sequential/        # Sequential solver
│   ├── openmp/           # OpenMP parallel solver
│   └── openmpi/          # OpenMPI distributed solver
├── scripts/               # Build and benchmark scripts
├── results/               # Performance data (CSV)
├── out/                   # Compiled binaries
└── README.md
```

## 🔬 Methodology

### Experimental Setup
- **Hardware**: Apple M2 MacBook Air (8-core CPU, 16GB RAM)
- **Software**: GCC 11.2, OpenMP 4.5, OpenMPI 4.1
- **Datasets**: Seven benchmark datasets from 85 to 140 items, including two subset-sum instances
- **Metrics**: Execution time, speedup factors, solution optimality verification

### Implementation Details
- **Sequential Baseline**: Single-threaded branch-and-bound implementation
- **OpenMP Version**: Shared-memory parallelization with task-based work distribution
- **OpenMPI Version**: Distributed-memory parallelization across multiple processes
- **Build System**: Automated compilation scripts with appropriate optimization flags

## 📈 Results & Discussion

### Performance Comparison
The experimental results validate the effectiveness of parallel computing approaches for the knapsack problem on Apple Silicon:

**OpenMP**: Best measured speedup is 7.94x at 4 threads on the 130-item subset-sum instance, against a theoretical ceiling of 8x on an 8-core machine. Speedup tracks problem size: instances that finish in ~100 ms show almost none, because thread setup and the shared bound update cost about as much as the work saved.

**OpenMPI**: Generally slower than the sequential baseline. Each rank walks the same node count on the smaller instances, so the run pays communication cost without dividing the search. It only beats sequential on the 121-item and subset-sum instances, and never beats OpenMP.

**Where the speedup comes from**: Not from more cores alone. It comes from a deeper search tree giving the shared incumbent bound time to prune. That is why the two slowest sequential instances are the only ones with meaningful gains.

**Correctness**: All seven datasets produce identical optimal values across sequential, OpenMP and OpenMPI runs.

### Algorithmic Insights
- **Optimality Preservation**: All parallel implementations maintain solution optimality through careful synchronization of global bounds
- **Memory Efficiency**: The branch-and-bound approach enables processing of large datasets (up to 30,000 items) within reasonable memory constraints
- **Load Balancing**: Effective work distribution strategies prevent processor idle time and maximize parallel efficiency

## 👤 Author

**Alaqmar G. and Connor B.**  
*Department of Computer Science, Brock University*  
*COSC 3P93: Parallel Computing*

## 📚 References

1. Martello, S., & Toth, P. (1990). *Knapsack Problems: Algorithms and Computer Implementations*. John Wiley & Sons.

2. OpenMP Architecture Review Board. (2021). *OpenMP 5.2 Specification*. https://www.openmp.org/

3. The Open MPI Project. (2021). *Open MPI: Open Source High Performance Computing*. https://www.open-mpi.org/

4. Horowitz, E., & Sahni, S. (1974). Computing partitions with applications to the knapsack problem. *Journal of the ACM*, 21(2), 277-292.

5. Pisinger, D. (2005). Where are the hard knapsack problems? *Computers & Operations Research*, 32(9), 2271-2284.

---

<p align="center">
  <strong>Academic Project - Department of Computer Science</strong><br>
  <strong>Brock University - COSC 3P93: Parallel Computing</strong><br>
  <em>Submitted for course requirements and evaluation</em>
</p>