#ifndef DATATYPES_H
#define DATATYPES_H

typedef struct {
    int id;
    float value;
    float weight;
} Item;

Item* read_items(const char* filename, int* count, float* capacity);

#endif