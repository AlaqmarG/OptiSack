#ifndef TREE_NODE_H
#define TREE_NODE_H

#include "parser/parser.h"

/**
 * TreeNode represents a state in the branch and bound search tree.
 * Each node corresponds to a decision point: include or exclude an item.
 * The tree structure allows exploration of all possible item combinations.
 */
class TreeNode {
public:
    Item item;              // Current item being considered at this node
    float t_weight;         // Total weight of items selected so far
    float t_value;          // Total value of items selected so far
    float bound;            // Upper bound on achievable value from this node
    int level;              // Depth in decision tree (item index)
    TreeNode* left;         // Left child: include current item
    TreeNode* right;        // Right child: exclude current item
    bool included;          // Whether this node represents an inclusion decision
    
    TreeNode() {
        item.id = 0;
        item.value = 0.0f;
        item.weight = 0.0f;
        t_weight = 0.0f;
        t_value = 0.0f;
        bound = 0.0f;
        level = -1;
        left = nullptr;
        right = nullptr;
        included = false;
    }
    
    /**
     * Creates and attaches a child node to this node.
     * 
     * @param isleft True for left child (include item), false for right (exclude item)
     * @param item_to_add The item being considered at the new node
     * @param new_t_weight Accumulated weight at the new node
     * @param new_t_value Accumulated value at the new node
     * @param new_level Depth level of the new node
     * @return Pointer to the newly created child node
     */
    TreeNode* add(bool isleft, Item item_to_add, float new_t_weight, float new_t_value, int new_level) {
        TreeNode* node = new TreeNode();
        node->item = item_to_add;
        node->t_weight = new_t_weight;
        node->t_value = new_t_value;
        node->level = new_level;
        node->included = isleft;
        
        if (isleft) {
            left = node;
        } else {
            right = node;
        }
        
        return node;
    }
    
    ~TreeNode() {
        if (left) delete left;
        if (right) delete right;
    }
};

#endif // TREE_NODE_H