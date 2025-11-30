#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <fstream>
#include <vector>
#include <algorithm>
#include "parser/parser.h"
#include "branch_and_bound_mpi.h"
#include "test_config.h"

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int world_rank = 0;
    int world_size = 1;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    const int ITERATIONS = 10;

    int item_count = 0;
    float capacity = 0.0f;
    Item* initial_items = read_items(TEST_FILE, &item_count, &capacity);
    if (initial_items == nullptr) {
        if (world_rank == 0) {
            printf("Error: Failed to load items from file %s\n", TEST_FILE);
        }
        MPI_Finalize();
        return 1;
    }

    std::vector<Item> template_items(initial_items, initial_items + item_count);
    delete[] initial_items;

    if (world_rank == 0) {
        printf("OpenMPI Branch and Bound Benchmark\n");
        printf("============================================\n");
        printf("Dataset: %s\n", TEST_FILE);
        printf("Processes: %d\n", world_size);
        printf("Iterations: %d\n\n", ITERATIONS);
        printf("Running benchmark...\n");
    }

    double total_time = 0.0;
    float final_max_value = 0.0f;
    long long total_nodes_explored = 0;
    long long total_nodes_pruned = 0;

    for (int iter = 0; iter < ITERATIONS; ++iter) {
        Item* items_copy = new Item[item_count];
        std::copy(template_items.begin(), template_items.end(), items_copy);

        MPI_Barrier(MPI_COMM_WORLD);
        double start = MPI_Wtime();

        float max_value = 0.0f;
        Item* best_items = nullptr;
        int best_count = 0;
        MpiStats stats{0, 0};
        branch_and_bound_mpi(items_copy,
                             item_count,
                             capacity,
                             &max_value,
                             &best_items,
                             &best_count,
                             &stats,
                             MPI_COMM_WORLD);

        double end = MPI_Wtime();
        double elapsed = end - start;

        delete[] items_copy;
        delete[] best_items;

        if (world_rank == 0) {
            total_time += elapsed;
            final_max_value = max_value;
            total_nodes_explored += stats.nodes_explored;
            total_nodes_pruned += stats.nodes_pruned;
            printf("  Completed %d/%d iterations...\n", iter + 1, ITERATIONS);
        }
    }

    if (world_rank == 0) {
        double avg_time = total_time / ITERATIONS;
        printf("\n============================================\n");
        printf("RESULTS\n");
        printf("============================================\n");
        printf("Total time (%d runs): %.3f seconds (%.1f ms)\n", ITERATIONS, total_time, total_time * 1000.0);
        printf("Average time per run: %.4f seconds (%.2f ms)\n", avg_time, avg_time * 1000.0);
        printf("Optimal value: %.2f\n", final_max_value);
        printf("Total nodes explored (5 runs): %lld\n", total_nodes_explored);
        printf("Total nodes pruned (5 runs):   %lld\n", total_nodes_pruned);
        printf("============================================\n");

        std::string csv_file = "results/openmpi_benchmarks.csv";
        std::ofstream csv(csv_file, std::ios::app);
        if (csv.is_open()) {
            csv << TEST_FILE << ",openmpi," << world_size << "," << ITERATIONS << ","
                << total_time << "," << avg_time << ","
                << total_nodes_explored << "," << total_nodes_pruned << ","
                << final_max_value << "\n";
        } else {
            printf("Warning: Could not open %s for writing\n", csv_file.c_str());
        }
    }

    MPI_Finalize();
    return 0;
}
