#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Parses the input file to read items and knapsack capacity.
 * File format:
 * - First line: integer N (number of items)
 * - Next N lines: id value weight (space-separated floats)
 * - Last line: capacity (float)
 * 
 * @param filename Path to the input file
 * @param item_count Output: number of items read
 * @param capacity Output: knapsack capacity
 * @return Array of items, or nullptr on error
 */
Item* read_items(const char* filename, int* item_count, float* capacity) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error: Cannot open file %s\n", filename);
        return nullptr;
    }
    
    // Read number of items
    if (fscanf(file, "%d", item_count) != 1) {
        printf("Error: Failed to read item count\n");
        fclose(file);
        return nullptr;
    }
    
    int n = *item_count;
    Item* items = new Item[n];
    
    // Read each item
    for (int i = 0; i < n; i++) {
        if (fscanf(file, "%d %f %f", &items[i].id, &items[i].value, &items[i].weight) != 3) {
            printf("Error: Failed to read item %d\n", i);
            delete[] items;
            fclose(file);
            return nullptr;
        }
    }
    
    // Read capacity
    if (fscanf(file, "%f", capacity) != 1) {
        printf("Error: Failed to read capacity\n");
        delete[] items;
        fclose(file);
        return nullptr;
    }
    
    fclose(file);
    return items;
}