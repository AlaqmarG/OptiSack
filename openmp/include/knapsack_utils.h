#ifndef KNAPSACK_UTILS_H
#define KNAPSACK_UTILS_H

#include "parser/parser.h"
#include "tree_node.h"

// Comparator to sort items by value-to-weight ratio (descending)
bool compare_items(const Item& a, const Item& b);

// Calculate upper bound on value for a node
float calculate_bound(TreeNode* node, Item* items, int n, float capacity);

#endif // KNAPSACK_UTILS_H

