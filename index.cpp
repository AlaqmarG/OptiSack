/**
 * COSC 3P93 - Project Step 2
 * Sequential Branch and Bound Implementation for 0/1 Knapsack Problem
 * 
 * This program solves the 0/1 knapsack problem using a branch and bound algorithm.
 * The algorithm explores a binary decision tree where each level represents an item,
 * and branches represent include/exclude decisions.
 */

#include <stdio.h>
#include <stdlib.h>
#include "parser/parser.h"
#include "branch_and_bound.h"

int main() {
    // Load items from input file
    int item_count = 0;
    Item* items = read_items("data/slow.txt", &item_count);
    
    printf("Loaded %d items from data file.\n", item_count);
    
    // Define knapsack capacity constraint
    float capacity = 5000.0f;
    printf("\nKnapsack capacity: %.2f\n", capacity);
    
    // Initialize solution variables
    float max_value = 0.0f;
    Item* best_items = nullptr;
    int best_count = 0;
    
    // Execute branch and bound algorithm
    TreeNode* solution_tree = branch_and_bound(items, item_count, capacity, 
                                               &max_value, &best_items, &best_count);
    
    // Display optimal solution
    printf("\n========== OPTIMAL SOLUTION ==========\n");
    printf("Maximum value: %.2f\n", max_value);
    printf("Number of items selected: %d\n", best_count);
    printf("\nItems in knapsack:\n");
    
    float total_weight = 0.0f;
    for (int i = 0; i < best_count; i++) {
        printf("  %s (value: %.2f, weight: %.2f)\n", 
               best_items[i].name, best_items[i].value, best_items[i].weight);
        total_weight += best_items[i].weight;
    }
    
    printf("\nTotal weight: %.2f / %.2f\n", total_weight, capacity);
    printf("======================================\n");
    
    // Free dynamically allocated memory
    delete solution_tree;
    delete[] best_items;
    delete[] items;
    
    return 0;
}