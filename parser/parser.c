#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    char *name;
    float value;
    float weight;
} Item;

Item* read_items(const char* filename, int* count) {
    FILE *file = fopen(filename, "r");

    char line[256];
    Item *items = malloc(50 * sizeof(Item));
    
    *count = 0;
    
    // Skip header line
    fgets(line, sizeof(line), file);
    
    while (fgets(line, sizeof(line), file) != NULL && *count < 50) {
        char *token = strtok(line, ":");
        items[*count].name = strdup(token);
        token = strtok(NULL, ":");
        items[*count].value = strtod(token, NULL);
        token = strtok(NULL, ":");
        items[*count].weight = strtod(token, NULL);
        (*count)++;
    }
    
    fclose(file);
    return items;
}