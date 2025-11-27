# OptiSack 🏆

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++11](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B11)
[![OpenMP](https://img.shields.io/badge/OpenMP-4.5-red.svg)](https://www.openmp.org/)
[![OpenMPI](https://img.shields.io/badge/OpenMPI-4.1-orange.svg)](https://www.open-mpi.org/)

High-performance parallel implementations of the 0/1 Knapsack Problem using branch-and-bound algorithms. We've got sequential, shared-memory (OpenMP), and distributed-memory (OpenMPI) versions all benchmarked against each other.

## ✨ Features

- 🚀 **Multiple Parallel Approaches**: Sequential baseline, OpenMP for shared memory, OpenMPI for distributed computing
- 📊 **Real Benchmarking**: 6 different datasets with automated performance testing
- 🎯 **Actually Optimal**: Guaranteed optimal solutions (not approximations)
- 📈 **Performance Tracking**: Detailed stats and CSV exports for analysis
- 🔧 **Cross-Platform**: Works on macOS/Linux with auto dependency detection
- 📚 **Well Documented**: Lots of examples and algorithm explanations

## 📋 Table of Contents

- [Quick Start](#-quick-start)
- [Installation](#-installation)
- [Usage](#-usage)
- [Performance](#-performance)
- [Algorithm Details](#-algorithm-details)
- [Project Structure](#-project-structure)
- [Contributing](#-contributing)
- [License](#-license)

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

## 📊 Performance

### Real Results (8-core system)

| Dataset | Sequential | OpenMP | OpenMPI | OpenMP Speedup | OpenMPI Speedup |
|---------|------------|--------|---------|----------------|-----------------|
| Fast (85 items) | 678ms | 3.3ms | 0.9ms | **206x** | **753x** |
| Medium (100 items) | 789ms | 4.0ms | 2.0ms | **197x** | **395x** |
| Very Hard (110 items) | 1850ms | 2.7ms | 5.9ms | **685x** | **314x** |
| Extreme (121 items) | 1100ms | 6.2ms | 11.4ms | **177x** | **96x** |
| Ultimate (121 items) | 4260ms | 17.6ms | 4.2ms | **242x** | **1014x** |

### What We Learned

- **OpenMP rocks** on single machines - up to 685x faster
- **OpenMPI scales** across multiple systems - up to 1014x faster
- **Both give optimal answers** - no approximations here
- **Memory efficient** - handles up to 30,000 items

## 🧠 Algorithm Details

### Branch and Bound

- **Bounds**: Uses fractional knapsack relaxation for tight upper bounds
- **Pruning**: Cuts off branches that can't beat current best
- **Search**: Best-first exploration with priority queues
- **Guarantee**: Always finds truly optimal solutions

### Parallel Approaches

#### OpenMP (Shared Memory)
- Task-based parallelism with `#pragma omp task`
- Syncs every 100 nodes to avoid overhead
- Different threads start from different points
- Uses locks for thread-safe best solution sharing

#### OpenMPI (Distributed Memory)
- Process-based across MPI ranks
- `MPI_Allreduce` for global best sync
- Each rank starts from different initial decisions
- Collective operations for stats

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

## 🤝 Contributing

Contributions welcome! Check out our [Contributing Guide](CONTRIBUTING.md) for details.

### Getting Started

```bash
# Fork and clone
git clone https://github.com/yourusername/OptiSack.git
cd OptiSack

# Make a feature branch
git checkout -b feature/your-idea

# Test your changes
./scripts/benchmark.sh openmp

# Send a pull request
```

### Code Guidelines

- Use C++11 features and RAII
- Comment complex algorithms
- Add performance tests for new features
- Update docs when changing APIs

## 📄 License

MIT License - see [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Based on classic branch-and-bound optimization
- Thanks to OpenMP and OpenMPI communities
- Academic research on knapsack algorithms

## 📞 Support

- **Issues**: [GitHub Issues](https://github.com/AlaqmarG/OptiSack/issues)
- **Docs**: Check inline comments and this README
- **Testing**: Run `./scripts/benchmark.sh` to verify setup

---

<p align="center">Built with ❤️ for high-performance computing and algorithm research</p>