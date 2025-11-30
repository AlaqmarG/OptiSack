/**
 * Benchmark harness for parallel branch and bound using OpenMP
 * Runs multiple iterations to get stable timing measurements
 */

#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <omp.h>
#include <string>
#include <fstream>
#include "parser/parser.h"
#include "branch_and_bound_parallel.h"
#include "test_config.h"

int main(int argc, char* argv[]) {
    // Parse thread count from command line or use default
    int num_threads = 4;  // Default to 4 threads
    if (argc > 1) {
        num_threads = atoi(argv[1]);
        if (num_threads < 1) {
            printf("Error: Thread count must be >= 1\n");
            return 1;
        }
    }
    
    const int ITERATIONS = 10;  // Run 10 times for measurements
    
    printf("Parallel Branch and Bound Benchmark (OpenMP)\n");
    printf("=============================================\n");
    printf("Dataset: %s\n", TEST_FILE);
    printf("Threads: %d\n", num_threads);
    printf("Iterations: %d\n\n", ITERATIONS);
    
    // Load items once
    int item_count = 0;
    float capacity = 0.0f;
    Item* items = read_items(TEST_FILE, &item_count, &capacity);
    if (items == nullptr) {
        printf("Error: Failed to load items from file.\n");
        return 1;
    }
    
    printf("Loaded %d items\n", item_count);
    printf("Capacity: %.2f\n\n", capacity);
    
    double total_time = 0.0;
    float final_max_value = 0.0f;
    long long total_nodes_explored = 0;
    long long total_nodes_pruned = 0;
    
    printf("Running benchmark...\n");
    
    for (int iter = 0; iter < ITERATIONS; iter++) {
        // Make a copy of items for each run (they get sorted)
        Item* items_copy = new Item[item_count];
        for (int i = 0; i < item_count; i++) {
            items_copy[i] = items[i];
        }
        
        float max_value = 0.0f;
        Item* best_items = nullptr;
        int best_count = 0;
        int iter_nodes_explored = 0;
        int iter_nodes_pruned = 0;
        
        // Time this iteration
        auto start = std::chrono::high_resolution_clock::now();
        
        TreeNode* solution_tree = branch_and_bound_parallel(
            items_copy, item_count, capacity, 
            &max_value, &best_items, &best_count, num_threads,
            &iter_nodes_explored, &iter_nodes_pruned);
        
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        
        total_time += elapsed.count();
        final_max_value = max_value;
        total_nodes_explored += iter_nodes_explored;
        total_nodes_pruned += iter_nodes_pruned;
        
        // Clean up
        delete solution_tree;
        delete[] best_items;
        delete[] items_copy;
        
        if ((iter + 1) % 1 == 0) {
            printf("  Completed %d/%d iterations...\n", iter + 1, ITERATIONS);
        }
    }
    
    delete[] items;
    
    double avg_time = total_time / ITERATIONS;
    double total_time_ms = total_time * 1000.0;
    double avg_time_ms = avg_time * 1000.0;
    
    printf("\n=============================================\n");
    printf("RESULTS\n");
    printf("=============================================\n");
    printf("Total time (%d runs): %.3f seconds (%.1f ms)\n", 
           ITERATIONS, total_time, total_time_ms);
    printf("Average time per run: %.4f seconds (%.2f ms)\n", 
           avg_time, avg_time_ms);
    printf("Optimal value: %.2f\n", final_max_value);
    printf("Total nodes explored (5 runs): %lld\n", total_nodes_explored);
    printf("Total nodes pruned (5 runs):   %lld\n", total_nodes_pruned);
    printf("\nFor comparison with sequential version:\n");
    printf("  Parallel (%d threads): %.3f s\n", num_threads, total_time);
    printf("  If sequential takes X seconds, speedup = X / %.3f\n", total_time);
    printf("=============================================\n");
    
    // Write results to CSV
    std::string csv_file = "results/openmp_benchmarks.csv";
    
    std::ofstream csv(csv_file, std::ios::app);
    if (!csv.is_open()) {
        printf("Warning: Could not open CSV file for writing\n");
        return 0;
    }
    
    // Extract dataset name from path
    std::string dataset_name = TEST_FILE;
    size_t last_slash = dataset_name.find_last_of("/");
    if (last_slash != std::string::npos) {
        dataset_name = dataset_name.substr(last_slash + 1);
    }
    
    csv << dataset_name << ",openmp," << num_threads << "," << ITERATIONS << ","
        << total_time << "," << avg_time << ","
        << total_nodes_explored << "," << total_nodes_pruned << ","
        << final_max_value << "\n";
    
    csv.close();
    printf("Results written to %s\n", csv_file.c_str());
    
    return 0;
}


