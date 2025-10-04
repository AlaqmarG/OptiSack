#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    char *name;
    float value;
    float weight;
} Item;

/**
 * Reads knapsack items from a file with format: Name:Value:Weight
 * 
 * @param filename Path to the data file
 * @param count Output parameter for number of items read
 * @return Dynamically allocated array of Item structures
 */
Item* read_items(const char* filename, int* count) {
    FILE *file = fopen(filename, "r");

    int max_items = 200;

    char line[256];
    Item *items = new Item[max_items];
    
    *count = 0;
    
    // Skip header line (Name:Value:Weight)
    fgets(line, sizeof(line), file);
    
    // Parse each line: tokenize by ':' delimiter
    while (fgets(line, sizeof(line), file) != NULL && *count < max_items) {
        // Extract item name
        char *token = strtok(line, ":");
        items[*count].name = new char[strlen(token) + 1];
        strcpy(items[*count].name, token);
        
        // Extract value
        token = strtok(NULL, ":");
        items[*count].value = strtod(token, NULL);
        
        // Extract weight
        token = strtok(NULL, ":");
        items[*count].weight = strtod(token, NULL);
        
        (*count)++;
    }
    
    fclose(file);
    return items;
}