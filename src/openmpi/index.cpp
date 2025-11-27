#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include "parser/parser.h"
#include "branch_and_bound_mpi.h"
#include "output_display.h"
#include "test_config.h"

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int world_rank = 0;
    int world_size = 1;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int item_count = 0;
    float capacity = 0.0f;
    Item* items = read_items(TEST_FILE, &item_count, &capacity);

    if (items == nullptr) {
        if (world_rank == 0) {
            printf("Error: Failed to load items from file %s\n", TEST_FILE);
        }
        MPI_Finalize();
        return 1;
    }

    if (world_rank == 0) {
        printf("Test file: %s\n", TEST_FILE);
        printf("Loaded %d items from data file.\n", item_count);
        printf("Knapsack capacity: %.2f\n", capacity);
        printf("MPI processes: %d\n\n", world_size);
    }

    float max_value = 0.0f;
    Item* best_items = nullptr;
    int best_count = 0;
    MpiStats stats{0, 0};

    branch_and_bound_mpi(items,
                         item_count,
                         capacity,
                         &max_value,
                         &best_items,
                         &best_count,
                         &stats,
                         MPI_COMM_WORLD);

    if (world_rank == 0) {
        printf("\n========== OPTIMAL SOLUTION (OpenMPI) =========\n");
        printf("Maximum value: %.2f\n", max_value);
        printf("Number of items selected: %d\n\n", best_count);

        float total_weight = 0.0f;
        for (int i = 0; i < best_count; ++i) {
            printf("  Item %d (value: %.2f, weight: %.2f)\n",
                   best_items[i].id,
                   best_items[i].value,
                   best_items[i].weight);
            total_weight += best_items[i].weight;
        }

        printf("\nTotal weight: %.2f / %.2f\n", total_weight, capacity);
        print_statistics(stats.nodes_explored, stats.nodes_pruned);
        printf("===========================================\n");
    }

    delete[] items;
    delete[] best_items;

    MPI_Finalize();
    return 0;
}
