#ifndef PARSER_H
#define PARSER_H

typedef struct {
    int id;
    float value;
    float weight;
} Item;

Item* read_items(const char* filename, int* item_count, float* capacity);

#endif // PARSER_H