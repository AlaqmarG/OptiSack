#ifndef BRANCH_AND_BOUND_PARALLEL_H
#define BRANCH_AND_BOUND_PARALLEL_H

#include <omp.h>
#include "parser/parser.h"
#include "tree_node.h"

/**
 * Comparator for priority queue to implement max-heap based on bound values.
 * Nodes with higher bounds are prioritized (best-first search strategy).
 */
struct CompareNode {
    bool operator()(TreeNode* a, TreeNode* b) {
        return a->bound < b->bound; // Returns true if a has lower priority than b
    }
};

// Reconstruct solution path from the tree
void reconstruct_solution(TreeNode* root, TreeNode* best_node, 
                         Item** best_items, int* best_count);

// OpenMP Parallel Branch and Bound algorithm for 0/1 Knapsack.
// Optional output parameters nodes_explored_out and nodes_pruned_out report
// how many nodes were visited and pruned during the search.
TreeNode* branch_and_bound_parallel(Item* items, int n, float capacity, 
                                   float* max_value, Item** best_items, 
                                   int* best_count, int num_threads,
                                   int* nodes_explored_out = nullptr,
                                   int* nodes_pruned_out = nullptr);

#endif // BRANCH_AND_BOUND_PARALLEL_H