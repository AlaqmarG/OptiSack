/**
 * Benchmark harness for sequential branch and bound
 * Runs multiple iterations to get stable timing measurements
 */

#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include "parser/parser.h"
#include "branch_and_bound.h"
#include "test_config.h"

int main() {
    const int ITERATIONS = 50;  // Run 50 times for stable measurements
    
    printf("Sequential Branch and Bound Benchmark\n");
    printf("======================================\n");
    printf("Dataset: %s\n", TEST_FILE);
    printf("Capacity: %.2f\n", CAPACITY);
    printf("Iterations: %d\n\n", ITERATIONS);
    
    // Load items once
    int item_count = 0;
    Item* items = read_items(TEST_FILE, &item_count);
    printf("Loaded %d items\n\n", item_count);
    
    double total_time = 0.0;
    float final_max_value = 0.0f;
    
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
        
        // Time this iteration
        auto start = std::chrono::high_resolution_clock::now();
        
        TreeNode* solution_tree = branch_and_bound(
            items_copy, item_count, CAPACITY, 
            &max_value, &best_items, &best_count
        );
        
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        
        total_time += elapsed.count();
        final_max_value = max_value;
        
        // Clean up
        delete solution_tree;
        delete[] best_items;
        delete[] items_copy;
        
        if ((iter + 1) % 10 == 0) {
            printf("  Completed %d/%d iterations...\n", iter + 1, ITERATIONS);
        }
    }
    
    delete[] items;
    
    double avg_time = total_time / ITERATIONS;
    double total_time_ms = total_time * 1000.0;
    double avg_time_ms = avg_time * 1000.0;
    
    printf("\n======================================\n");
    printf("RESULTS\n");
    printf("======================================\n");
    printf("Total time (%d runs): %.3f seconds (%.1f ms)\n", 
           ITERATIONS, total_time, total_time_ms);
    printf("Average time per run: %.4f seconds (%.2f ms)\n", 
           avg_time, avg_time_ms);
    printf("Optimal value: %.2f\n", final_max_value);
    printf("\nFor parallel comparison, use total time of %d iterations:\n", ITERATIONS);
    printf("  Sequential: %.3f s\n", total_time);
    printf("  If parallel takes X seconds, speedup = %.3f / X\n", total_time);
    printf("======================================\n");
    
    return 0;
}

