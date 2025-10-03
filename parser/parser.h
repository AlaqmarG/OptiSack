#ifndef DATATYPES_H
#define DATATYPES_H

typedef struct {
    char *name;
    float value;
    float weight;
} Item;

Item* read_items(const char* filename, int* count);

#endif