#include "knapsack_utils.h"

/**
 * Comparator to sort items by value-to-weight ratio in descending order.
 * When ratios are equal, sort by value descending for better bound quality.
 */
bool compare_items(const Item& a, const Item& b) {
    float ra = a.value / a.weight;
    float rb = b.value / b.weight;
    if (ra != rb) return ra > rb;
    return a.value > b.value;
}

/**
 * Calculates the upper bound on maximum value achievable from a given node.
 * Uses fractional knapsack relaxation: greedily adds items until capacity is full,
 * allowing fractional items to get an optimistic estimate.
 * 
 * This bound is used for pruning: if bound <= current_best, the branch can be eliminated.
 * 
 * @param node Current node in the search tree
 * @param items Array of items sorted by value/weight ratio
 * @param n Total number of items
 * @param capacity Maximum knapsack capacity
 * @return Upper bound on value for this branch
 */
float calculate_bound(TreeNode* node, Item* items, int n, float capacity) {
    // If we've exceeded capacity, this branch is infeasible
    if (node->t_weight >= capacity) {
        return 0.0f;
    }
    
    float bound = node->t_value;
    int j = node->level + 1;
    float total_weight = node->t_weight;
    
    // Greedily add complete items while they fit
    while (j < n && total_weight + items[j].weight <= capacity) {
        total_weight += items[j].weight;
        bound += items[j].value;
        j++;
    }
    
    // Add fractional part of next item (relaxation of 0/1 constraint)
    // This gives an optimistic upper bound for pruning
    if (j < n) {
        bound += (capacity - total_weight) * (items[j].value / items[j].weight);
    }
    
    return bound;
}