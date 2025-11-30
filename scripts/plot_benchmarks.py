#!/usr/bin/env python3
"""
Plot bar chart of speedup for each implementation on the benchmark datasets.

Reads:
  - results/sequential_benchmarks.csv
  - results/openmp_benchmarks.csv
  - results/openmpi_benchmarks.csv

Outputs:
  - writeup/images/speedup_by_impl.png
"""

import csv
import os
from collections import defaultdict

import matplotlib.pyplot as plt

ROOT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def load_sequential_totals():
    """Return total_time_sec for each dataset from the sequential baseline."""
    path = os.path.join(ROOT_DIR, "results", "sequential_benchmarks.csv")
    totals = {}
    with open(path, newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            dataset = row["dataset"]
            totals[dataset] = float(row["total_time_sec"])
    return totals


def load_openmp_speedups(seq_totals):
    """Return per-dataset mapping of thread count -> speedup."""
    path = os.path.join(ROOT_DIR, "results", "openmp_benchmarks.csv")
    speedups = defaultdict(list)  # dataset -> list[(threads, speedup)]
    if not os.path.exists(path):
        return speedups

    with open(path, newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            dataset = row["dataset"]
            if dataset not in seq_totals:
                continue
            threads = int(row["threads"])
            total_time = float(row["total_time_sec"])
            seq_time = seq_totals[dataset]
            s = seq_time / total_time if total_time > 0 else 0.0
            speedups[dataset].append((threads, s))

    for ds in speedups:
        speedups[ds].sort(key=lambda x: x[0])
    return speedups


def load_openmpi_speedups(seq_totals):
    """Return per-dataset mapping of process count -> speedup."""
    path = os.path.join(ROOT_DIR, "results", "openmpi_benchmarks.csv")
    speedups = defaultdict(list)  # dataset -> list[(procs, speedup)]
    if not os.path.exists(path):
        return speedups

    with open(path, newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            dataset = os.path.basename(row["dataset"])
            if dataset not in seq_totals:
                continue
            procs = int(row["processes"])
            total_time = float(row["total_time_sec"])
            seq_time = seq_totals[dataset]
            s = seq_time / total_time if total_time > 0 else 0.0
            speedups[dataset].append((procs, s))

    for ds in speedups:
        speedups[ds].sort(key=lambda x: x[0])
    return speedups


def load_best_configs():
    """
    Return per-dataset summary of best (fastest) configuration for each implementation.

    Result:
      summary[dataset] = {
          "sequential": {"time": ..., "nodes": ...},
          "openmp": {"time": ..., "nodes": ..., "threads": ...},
          "openmpi": {"time": ..., "nodes": ..., "procs": ...},
      }
    """
    summary = defaultdict(dict)

    # Sequential: single configuration per dataset.
    seq_path = os.path.join(ROOT_DIR, "results", "sequential_benchmarks.csv")
    if os.path.exists(seq_path):
        with open(seq_path, newline="") as f:
            reader = csv.DictReader(f)
            for row in reader:
                ds = row["dataset"]
                summary[ds]["sequential"] = {
                    "time": float(row["total_time_sec"]),
                    "nodes": float(row["nodes_explored"]),
                }

    # OpenMP: choose minimum total_time_sec per dataset.
    omp_path = os.path.join(ROOT_DIR, "results", "openmp_benchmarks.csv")
    if os.path.exists(omp_path):
        with open(omp_path, newline="") as f:
            reader = csv.DictReader(f)
            for row in reader:
                ds = row["dataset"]
                time = float(row["total_time_sec"])
                nodes = float(row["nodes_explored"])
                threads = int(row["threads"])
                best = summary[ds].get("openmp")
                if best is None or time < best["time"]:
                    summary[ds]["openmp"] = {
                        "time": time,
                        "nodes": nodes,
                        "threads": threads,
                    }

    # OpenMPI: choose minimum total_time_sec per dataset.
    mpi_path = os.path.join(ROOT_DIR, "results", "openmpi_benchmarks.csv")
    if os.path.exists(mpi_path):
        with open(mpi_path, newline="") as f:
            reader = csv.DictReader(f)
            for row in reader:
                ds = os.path.basename(row["dataset"])
                time = float(row["total_time_sec"])
                nodes = float(row["nodes_explored"])
                procs = int(row["processes"])
                best = summary[ds].get("openmpi")
                if best is None or time < best["time"]:
                    summary[ds]["openmpi"] = {
                        "time": time,
                        "nodes": nodes,
                        "procs": procs,
                    }

    return summary


def prettify_name(name: str) -> str:
    """Shorten dataset file names for plotting."""
    base = os.path.splitext(os.path.basename(name))[0]
    if base.startswith("benchmark_"):
        base = base[len("benchmark_") :]
    base = base.replace("_items", "")
    # If the name starts with a numeric prefix (e.g., "130_subset_sum"), use just that.
    parts = base.split("_")
    if parts and parts[0].isdigit():
        return parts[0]
    return base.replace("_", " ")


def items_label(name: str) -> str:
    """Return a short 'XX items' label extracted from the dataset name."""
    base = os.path.splitext(os.path.basename(name))[0]
    # Expect names like benchmark_fast_85items or 130_subset_sum
    parts = base.split("_")
    if parts and parts[0].isdigit():
        return f"{parts[0]} items"
    # Fallback to prettified name if pattern is unexpected.
    return prettify_name(name)


def plot_speedup_per_dataset(seq_totals, omp_speedups, mpi_speedups):
    """Plot per-dataset speedup vs. thread / process count (existing multi-panel figure)."""
    datasets = sorted(seq_totals.keys())

    num_plots = len(datasets)
    if num_plots == 1:
        nrows, ncols = 1, 1
    elif num_plots <= 4:
        nrows, ncols = 2, 2
    else:
        ncols = 2
        nrows = (num_plots + 1) // 2

    fig, axes = plt.subplots(nrows, ncols, figsize=(7, 2.5 * nrows), sharey=True)

    # Normalize axes to a flat list for iteration.
    if nrows * ncols == 1:
        axes = [axes]
    else:
        axes = axes.ravel()

    for ax, ds in zip(axes, datasets):
        ax.axhline(
            1.0,
            color="black",
            linestyle="--",
            linewidth=1,
            label="Sequential (1×)" if ds == datasets[0] else "",
        )

        omp_data = omp_speedups.get(ds, [])
        mpi_data = mpi_speedups.get(ds, [])

        omp_dict = {t: s for (t, s) in omp_data}
        mpi_dict = {p: s for (p, s) in mpi_data}
        all_counts = sorted(set(omp_dict.keys()) | set(mpi_dict.keys()))

        indices = list(range(len(all_counts)))
        width = 0.35

        if omp_dict:
            omp_vals = [omp_dict.get(c, 0.0) for c in all_counts]
            ax.bar(
                [i - width / 2 for i in indices],
                omp_vals,
                width=width,
                color="tab:blue",
                label="OpenMP",
            )

        if mpi_dict:
            mpi_vals = [mpi_dict.get(c, 0.0) for c in all_counts]
            ax.bar(
                [i + width / 2 for i in indices],
                mpi_vals,
                width=width,
                color="tab:orange",
                label="OpenMPI",
            )

        ax.set_title(f"Speedup vs Sequential for {items_label(ds)}")
        ax.set_xlabel("Threads / Processes")
        ax.set_ylabel("Speedup (T_seq / T_par)")
        ax.set_xticks(indices)
        ax.set_xticklabels([str(c) for c in all_counts])
        ax.grid(axis="y", linestyle=":", linewidth=0.5)

    # Hide any unused subplots in the grid.
    for ax in axes[len(datasets) :]:
        ax.axis("off")

    handles, labels = axes[0].get_legend_handles_labels()
    if handles:
        fig.legend(handles, labels, loc="lower center", ncol=len(labels))
        plt.tight_layout(rect=[0, 0.05, 1, 0.98])
    else:
        plt.tight_layout()

    out_dir = os.path.join(ROOT_DIR, "writeup", "images")
    os.makedirs(out_dir, exist_ok=True)
    out_path = os.path.join(out_dir, "speedup_by_impl.png")
    fig.savefig(out_path, dpi=200)
    print(f"Wrote plot to {out_path}")


def plot_summary_bars(best_configs):
    """
    Plot summary bar charts for total time and nodes explored using best configs.

    For each dataset:
      - sequential (1 thread)
      - best OpenMP configuration
      - best OpenMPI configuration
    """
    datasets = sorted(best_configs.keys())
    labels = [prettify_name(ds) for ds in datasets]

    impls = ["sequential", "openmp", "openmpi"]
    impl_labels = ["Seq (1 thread)", "OpenMP (best)", "OpenMPI (best)"]
    colors = ["tab:gray", "tab:blue", "tab:orange"]

    # Collect data
    times = {impl: [] for impl in impls}
    nodes = {impl: [] for impl in impls}

    for ds in datasets:
        cfg = best_configs[ds]
        for impl in impls:
            info = cfg.get(impl)
            if info is None:
                times[impl].append(0.0)
                nodes[impl].append(0.0)
            else:
                times[impl].append(info["time"])
                nodes[impl].append(info["nodes"] / 1e6)  # scale to millions

    x = list(range(len(datasets)))
    width = 0.25

    # Total time figure
    fig_time, ax_time = plt.subplots(figsize=(8, 3.5))
    for i, impl in enumerate(impls):
        offsets = [xi + (i - 1) * width for xi in x]
        ax_time.bar(offsets, times[impl], width=width, color=colors[i], label=impl_labels[i])
    ax_time.set_xticks(x)
    ax_time.set_xticklabels(labels, rotation=15)
    ax_time.set_ylabel("Total time over 5 runs (s)")
    ax_time.set_title("Total time for best configuration per implementation")
    ax_time.grid(axis="y", linestyle=":", linewidth=0.5)
    ax_time.legend()
    fig_time.tight_layout()

    out_dir = os.path.join(ROOT_DIR, "writeup", "images")
    os.makedirs(out_dir, exist_ok=True)
    time_path = os.path.join(out_dir, "time_by_impl.png")
    fig_time.savefig(time_path, dpi=200)
    print(f"Wrote plot to {time_path}")

    # Nodes figure
    fig_nodes, ax_nodes = plt.subplots(figsize=(8, 3.5))
    for i, impl in enumerate(impls):
        offsets = [xi + (i - 1) * width for xi in x]
        ax_nodes.bar(offsets, nodes[impl], width=width, color=colors[i], label=impl_labels[i])
    ax_nodes.set_xticks(x)
    ax_nodes.set_xticklabels(labels, rotation=15)
    ax_nodes.set_ylabel("Nodes explored (millions, 5 runs)")
    ax_nodes.set_title("Nodes explored for best configuration per implementation")
    ax_nodes.grid(axis="y", linestyle=":", linewidth=0.5)
    ax_nodes.legend()
    fig_nodes.tight_layout()

    nodes_path = os.path.join(out_dir, "nodes_by_impl.png")
    fig_nodes.savefig(nodes_path, dpi=200)
    print(f"Wrote plot to {nodes_path}")


def main():
    seq_totals = load_sequential_totals()
    if not seq_totals:
        print("No sequential benchmark data found; nothing to plot.")
        return

    omp_speedups = load_openmp_speedups(seq_totals)
    mpi_speedups = load_openmpi_speedups(seq_totals)

    # Per-dataset speedup vs. threads/processes (multi-panel figure).
    plot_speedup_per_dataset(seq_totals, omp_speedups, mpi_speedups)

    # Summary bar charts for time and nodes using best configurations.
    best_configs = load_best_configs()
    if best_configs:
        plot_summary_bars(best_configs)


if __name__ == "__main__":
    main()


