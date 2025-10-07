#include <stdio.h>
#include "output_display.h"

/**
 * Displays items sorted by value/weight ratio.
 * This ordering is crucial for the branch and bound algorithm's effectiveness.
 */
void print_sorted_items(Item* items, int n) {
    printf("\nSorted items by value/weight ratio:\n");
    for (int i = 0; i < n; i++) {
        printf("%d. Item %d: value=%.2f, weight=%.2f, ratio=%.4f\n", 
               i+1, items[i].id, items[i].value, items[i].weight, 
               items[i].value / items[i].weight);
    }
}

/**
 * Displays branch and bound algorithm performance metrics.
 * Nodes pruned indicates the effectiveness of the bounding function.
 */
void print_statistics(int nodes_explored, int nodes_pruned) {
    printf("\nBranch and Bound Statistics:\n");
    printf("Nodes explored: %d\n", nodes_explored);
    printf("Nodes pruned: %d\n", nodes_pruned);
}

