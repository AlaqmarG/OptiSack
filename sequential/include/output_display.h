#ifndef OUTPUT_DISPLAY_H
#define OUTPUT_DISPLAY_H

#include "parser/parser.h"

// Print sorted items information
void print_sorted_items(Item* items, int n);

// Print algorithm statistics
void print_statistics(int nodes_explored, int nodes_pruned);

#endif // OUTPUT_DISPLAY_H

