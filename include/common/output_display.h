#ifndef OUTPUT_DISPLAY_H
#define OUTPUT_DISPLAY_H

#include "parser/parser.h"
#include "tree_node.h"

void print_sorted_items(Item* items, int n);
void print_statistics(int nodes_explored, int nodes_pruned);

#endif // OUTPUT_DISPLAY_H