<div align="center">

# OptiSack

### Parallel Branch-and-Bound 0/1 Knapsack Solver
Department of Computer Science, Brock University · COSC 3P93 — Parallel Computing

---

## RESULT

# 7990x FASTER

**OpenMPI vs. sequential** on the 112-item medium-hard instance — same provably optimal answer, not an approximation.

<sub>Pulled directly from <code>results/openmpi_benchmarks.csv</code> and <code>results/sequential_benchmarks.csv</code> — full scoreboard below.</sub>

---

<p>
  <a href="https://opensource.org/licenses/MIT"><img src="https://img.shields.io/badge/License-MIT-00F5A0.svg?style=flat-square&labelColor=0B0F19" alt="License"></a>
  <a href="https://en.wikipedia.org/wiki/C%2B%2B11"><img src="https://img.shields.io/badge/C%2B%2B-11-00D9F5.svg?style=flat-square&labelColor=0B0F19" alt="C++"></a>
  <a href="https://www.openmp.org/"><img src="https://img.shields.io/badge/OpenMP-4.5-00F5A0.svg?style=flat-square&labelColor=0B0F19" alt="OpenMP"></a>
  <a href="https://www.open-mpi.org/"><img src="https://img.shields.io/badge/OpenMPI-4.1-00D9F5.svg?style=flat-square&labelColor=0B0F19" alt="OpenMPI"></a>
</p>

</div>

---

## Scoreboard

Same branch-and-bound algorithm, same optimal answer, three execution models — sequential, OpenMP (shared memory), OpenMPI (distributed memory) — benchmarked on an Apple M2 MacBook Air (8-core, 16GB RAM):

| Dataset | Sequential | OpenMP | OpenMPI | OpenMP Speedup | OpenMPI Speedup |
|---|---:|---:|---:|---:|---:|
| Fast (85 items) | 733ms | 2.0ms | 1.3ms | **367x** | **611x** |
| Medium (100 items) | 227ms | 3.2ms | 1.1ms | **71x** | **189x** |
| Medium-Hard (112 items) | 14381ms | 3.1ms | 1.8ms | **4794x** | **7990x** |
| Very Hard (110 items) | 1670ms | 3.3ms | 6.2ms | **522x** | **269x** |
| Extreme (121 items) | 711ms | 4.6ms | 12.0ms | **155x** | **59x** |
| Ultimate (121 items) | 2176ms | 5.1ms | 3.5ms | **435x** | **605x** |

Every parallel run still returns the **provably optimal** knapsack solution — parallelism buys speed, not approximation.

```
 Sequential   ██████████████████████████████████████████████████  14381ms  (112-item worst case)
 OpenMP       ▏                                                        3.1ms
 OpenMPI      ▏                                                        1.8ms
```

## Why the numbers swing around

The speedup isn't monotonic with problem size, and that's the interesting part, not a flaw:

- **Medium-Hard (112 items) is where it gets dramatic.** This instance has a shape that makes the sequential search explore a huge number of nodes before pruning kicks in — 14.4 seconds worth. Splitting the initial decision tree across threads/ranks lets each worker prune independently and in parallel, collapsing that to single-digit milliseconds. That's the 4794x/7990x outlier.
- **OpenMPI edges out OpenMP on easy instances, loses on hard ones.** Distributed ranks avoid shared-memory contention entirely (611x vs 367x on the 85-item set), but `MPI_Allreduce` synchronization and process-spawn overhead cost more as the search tree gets deeper and more irregular — visible on Extreme (121 items), where OpenMPI's 59x trails OpenMP's 155x.
- **More cores isn't automatically faster.** Raw thread/process-count sweeps in `results/openmp_benchmarks.csv` and `results/openmpi_benchmarks.csv` show wall-clock time *increasing* past 2–4 workers on several datasets — branch-and-bound's irregular, data-dependent tree makes load balancing genuinely hard, and synchronization overhead compounds with worker count.

## Three parallel paradigms, one problem

| | Sequential | OpenMP | OpenMPI |
|---|---|---|---|
| Memory model | — | Shared | Distributed |
| Parallel unit | — | `#pragma omp task` per subtree | MPI rank per subtree |
| Sync strategy | — | Periodic sync every 100 nodes, locked global best | `MPI_Allreduce` for global best |
| Best for | Baseline correctness | Single-node, low-latency sharing | Multi-node scale-out |

**Branch-and-bound core:** fractional-knapsack relaxation for tight upper bounds, best-first search via priority queue, and aggressive pruning wherever a branch's bound can't beat the current best — the same guarantee-of-optimality machinery underneath all three implementations.

---

<details>
<summary><strong>Appendix: installation, methodology, datasets, project structure, references</strong></summary>

## Quick start

```bash
git clone https://github.com/AlaqmarG/OptiSack.git && cd OptiSack

./scripts/run.sh benchmark_medium_100items.txt openmp      # auto-detects cores
./scripts/benchmark.sh sequential openmp openmpi           # full suite -> results/*.csv
```

Needs a C++11 compiler (GCC 7+/Clang 5+), `libomp`, and `openmpi` (`brew install libomp openmpi` on macOS).

## Datasets

| Dataset | Items | Difficulty |
|---|---:|---|
| `benchmark_fast_85items.txt` | 85 | Easy |
| `benchmark_medium_100items.txt` | 100 | Medium |
| `benchmark_medium_hard_112items.txt` | 112 | Hard |
| `benchmark_very_hard_110items.txt` | 110 | Very Hard |
| `benchmark_extreme_121items.txt` | 121 | Extreme |
| `benchmark_ultimate_121items.txt` | 121 | Ultimate |

Branch-and-bound's search space grows combinatorially with item count, so these were hand-picked to stress the pruning logic at different tree shapes, not just to scale up size — the biggest speedups above come from a 112-item set, not the 121-item ones.

## Methodology

- **Hardware:** Apple M2 MacBook Air, 8-core CPU, 16GB RAM
- **Software:** GCC 11.2, OpenMP 4.5, OpenMPI 4.1
- **Process:** each dataset run 10 iterations per configuration (see `results/*.csv` for raw per-run node counts and timings), optimal value cross-checked across all three implementations to confirm correctness before comparing speed

## Project structure

```
OptiSack/
├── data/         # Benchmark datasets
├── include/      # Headers: common/ sequential/ openmp/ openmpi/
├── src/          # Implementations, mirroring include/
├── scripts/      # run.sh, benchmark.sh, plot_benchmarks.py
└── results/      # Raw CSV benchmark output
```

## References

1. Martello, S., & Toth, P. (1990). *Knapsack Problems: Algorithms and Computer Implementations*. John Wiley & Sons.
2. OpenMP Architecture Review Board. (2021). *OpenMP 5.2 Specification*.
3. The Open MPI Project. (2021). *Open MPI: Open Source High Performance Computing*.
4. Horowitz, E., & Sahni, S. (1974). Computing partitions with applications to the knapsack problem. *Journal of the ACM*, 21(2), 277-292.
5. Pisinger, D. (2005). Where are the hard knapsack problems? *Computers & Operations Research*, 32(9), 2271-2284.

</details>

---

<div align="center">

**Alaqmar G. and Connor B.** · Department of Computer Science, Brock University · COSC 3P93

</div>
