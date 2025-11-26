#include "output_display.h"
#include <stdio.h>

/**
 * Prints the sorted list of items with their ratios.
 * Used for debugging and verification of the sorting process.
 * 
 * @param items Array of sorted items
 * @param n Number of items
 */
void print_sorted_items(Item* items, int n) {
    printf("\nSorted items by value/weight ratio:\n");
    for (int i = 0; i < n; i++) {
        printf("%d. Item %d: value=%.2f, weight=%.2f, ratio=%.4f\n", 
               i + 1, items[i].id, items[i].value, items[i].weight, 
               items[i].value / items[i].weight);
    }
    printf("\n");
}

/**
 * Prints statistics about the branch and bound search.
 * Shows the number of nodes explored and pruned during the search.
 * 
 * @param nodes_explored Total number of nodes visited
 * @param nodes_pruned Number of nodes pruned due to bound checks
 */
void print_statistics(int nodes_explored, int nodes_pruned) {
    printf("\nBranch and Bound Statistics:\n");
    printf("Nodes explored: %d\n", nodes_explored);
    printf("Nodes pruned: %d\n", nodes_pruned);
}