# OptiSack: 0/1 Knapsack Solver

A high-performance C++ implementation of the 0/1 knapsack problem using branch and bound algorithm with sequential, OpenMP, and OpenMPI parallel implementations.

## Features

- **Branch and Bound Algorithm**: Uses fractional relaxation bounds for efficient pruning
- **Parallel Implementations**: OpenMP for shared-memory systems and OpenMPI for distributed-memory execution
- **Comprehensive Benchmarks**: Multiple test datasets with varying difficulty
- **CSV Output**: Automatic results logging for analysis
- **Optimized Performance**: Improved sorting and bound calculations

## Project Structure

```
OptiSack/
├── data/                    # Benchmark datasets
├── include/common/          # Shared headers
├── src/
│   ├── common/             # Shared utilities
│   ├── sequential/         # Sequential implementation
│   ├── openmp/             # OpenMP implementation
│   └── openmpi/            # OpenMPI implementation
├── scripts/                # Build and run scripts
├── results/                # CSV benchmark results
└── out/                    # Compiled binaries
```

## Quick Start

### Single Dataset Test
```bash
# Sequential version
./scripts/run.sh benchmark_fast_85items.txt sequential

# OpenMP version (auto-detects CPU cores, defaults to 8 if detection fails)
./scripts/run.sh benchmark_fast_85items.txt openmp

# OpenMP version with specific thread count
./scripts/run.sh benchmark_fast_85items.txt openmp 4

# OpenMPI version (auto-detects process count when omitted)
./scripts/run.sh benchmark_fast_85items.txt openmpi

# OpenMPI version with explicit process count
./scripts/run.sh benchmark_fast_85items.txt openmpi 8
```

### Full Benchmark Suite
```bash
# Sequential benchmarks
./scripts/benchmark.sh sequential

# OpenMP benchmarks (auto-detects and uses all available CPU cores)
./scripts/benchmark.sh openmp

# OpenMPI benchmarks (auto-detects number of ranks)
./scripts/benchmark.sh openmpi

# All implementations
./scripts/benchmark.sh sequential openmp openmpi
```

## Benchmark Results

Results are automatically saved to CSV files in the `results/` directory:

- `results/sequential_benchmarks.csv`: Sequential implementation results
- `results/openmp_benchmarks.csv`: OpenMP implementation results
- `results/openmpi_benchmarks.csv`: OpenMPI implementation results

CSV Format:
```csv
dataset,implementation,threads,iterations,total_time_sec,avg_time_sec,optimal_value
benchmark_fast_85items.txt,sequential,1,5,3.389,0.678,2005.43
```

## Datasets

| Dataset | Items | Description |
|---------|-------|-------------|
| `benchmark_fast_85items.txt` | 85 | Fast convergence |
| `benchmark_medium_100items.txt` | 100 | Medium difficulty |
| `benchmark_medium_hard_112items.txt` | 112 | Medium-hard |
| `benchmark_very_hard_110items.txt` | 110 | Very hard |
| `benchmark_extreme_121items.txt` | 121 | Extreme difficulty |
| `benchmark_ultimate_121items.txt` | 121 | Ultimate challenge |

## Performance Notes

- **Benchmark Iterations**: Reduced to 5 iterations for faster testing while maintaining measurement quality
- **Parallel Speedup**: OpenMP accelerates shared-memory workloads while OpenMPI scales across processes
- **Thread/Rank Detection**: Automatically detects and utilizes available CPU cores or MPI ranks for optimal performance
- **Bound Quality**: Uses fractional relaxation bounds for better pruning efficiency
- **Memory Usage**: Optimized for datasets up to 30,000 items

## Algorithm Details

### Branch and Bound
- **Upper Bound**: Fractional knapsack relaxation
- **Lower Bound**: Current solution value
- **Pruning**: Nodes with upper bound ≤ current best are pruned
- **Search Strategy**: Best-first search using priority queue

### Parallelization
- **OpenMP**: Uses task-based parallelism with `#pragma omp task` for dynamic load balancing. Employs locks and critical sections for thread-safe access to shared best solution.
- **OpenMPI**: MPI approach - uses sequential execution on rank 0 for computation, with MPI used for broadcasting results to all processes.
- **Shared Bounds**: Global best solution tracking with atomic operations (OpenMP) or broadcasts (OpenMPI)
- **Thread/Process Safety**: Lock-based synchronization (OpenMP) and MPI collective operations ensure consistency

## Building

The build scripts automatically handle compilation with appropriate flags:

- **Sequential**: Standard C++11 compilation
- **OpenMP**: Includes OpenMP flags and library linking (macOS/homebrew compatible)
- **OpenMPI**: Uses `mpic++` (or `mpicc`) to compile MPI variants and launches them via `mpirun`

## Dependencies

- C++11 compatible compiler (GCC/Clang)
- OpenMP support (libomp on macOS)
- OpenMPI runtime (mpic++/mpirun)
- Standard C++ libraries

## Output Analysis

The solver provides detailed statistics:
- **Nodes Explored**: Total search tree nodes visited
- **Nodes Pruned**: Nodes eliminated by bound pruning
- **Optimal Value**: Maximum achievable value
- **Selected Items**: Complete solution listing

## Development

### Code Organization
- **Shared Code**: Common utilities in `src/common/`
- **Implementation Separation**: Clean separation between sequential and parallel versions
- **Modular Design**: Easy to extend with new algorithms or optimizations

### Testing
- **Correctness Verification**: All implementations find provably optimal solutions
- **Performance Validation**: Consistent timing across multiple runs
- **Cross-Implementation Consistency**: Sequential and parallel versions produce identical results