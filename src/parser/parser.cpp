#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    int id;
    float value;
    float weight;
} Item;

/**
 * Reads knapsack items from a file with format:
 * Line 1: n (number of items)
 * Next n lines: id value weight
 * Last line: capacity
 * 
 * @param filename Path to the data file
 * @param count Output parameter for number of items read
 * @param capacity Output parameter for knapsack capacity
 * @return Dynamically allocated array of Item structures
 */
Item* read_items(const char* filename, int* count, float* capacity) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open file %s\n", filename);
        *count = 0;
        *capacity = 0.0f;
        return nullptr;
    }

    // Read number of items from first line
    if (fscanf(file, "%d", count) != 1) {
        fprintf(stderr, "Error: Could not read number of items\n");
        fclose(file);
        *count = 0;
        *capacity = 0.0f;
        return nullptr;
    }

    // Allocate array for items
    Item *items = new Item[*count];
    
    // Read each item: id value weight
    for (int i = 0; i < *count; i++) {
        if (fscanf(file, "%d %f %f", &items[i].id, &items[i].value, &items[i].weight) != 3) {
            fprintf(stderr, "Error: Could not read item %d\n", i);
            delete[] items;
            fclose(file);
            *count = 0;
            *capacity = 0.0f;
            return nullptr;
        }
    }
    
    // Read capacity from last line
    if (fscanf(file, "%f", capacity) != 1) {
        fprintf(stderr, "Error: Could not read capacity\n");
        delete[] items;
        fclose(file);
        *count = 0;
        *capacity = 0.0f;
        return nullptr;
    }
    
    fclose(file);
    return items;
}