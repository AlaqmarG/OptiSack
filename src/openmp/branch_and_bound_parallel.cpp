#include <queue>
#include <algorithm>
#include <vector>
#include <functional>
#include <omp.h>
#include <atomic>
#include "branch_and_bound_parallel.h"
#include "knapsack_utils.h"
#include "output_display.h"

/**
 * Reconstructs the optimal solution by traversing the search tree.
 * Performs depth-first search from root to find the path leading to best_node,
 * collecting all items that were included along the path.
 * 
 * @param root Root of the search tree
 * @param best_node Node containing the optimal solution
 * @param best_items Output array to store selected items
 * @param best_count Output count of selected items
 */
void reconstruct_solution(TreeNode* root, TreeNode* best_node, 
                         Item** best_items, int* best_count) {
    std::vector<Item> solution_items;
    std::function<bool(TreeNode*, float, float, std::vector<Item>&)> find_path;
    
    // Recursive lambda to find path from root to target node
    // Matches nodes by their accumulated value and weight
    find_path = [&](TreeNode* node, float target_value, float target_weight, 
                     std::vector<Item>& path) -> bool {
        if (!node) return false;
        
        // Found the target node
        if (node->t_value == target_value && node->t_weight == target_weight) {
            return true;
        }
        
        // Explore left subtree (item inclusion branch)
        if (node->left) {
            if (node->left->included) {
                path.push_back(node->left->item);
            }
            if (find_path(node->left, target_value, target_weight, path)) {
                return true;
            }
            // Backtrack if path not found
            if (node->left->included) {
                path.pop_back();
            }
        }
        
        // Explore right subtree (item exclusion branch)
        if (node->right) {
            if (find_path(node->right, target_value, target_weight, path)) {
                return true;
            }
        }
        
        return false;
    };
    
    find_path(root, best_node->t_value, best_node->t_weight, solution_items);
    
    *best_count = solution_items.size();
    for (int i = 0; i < *best_count; i++) {
        (*best_items)[i] = solution_items[i];
    }
}

/**
 * Parallel Branch and Bound algorithm for 0/1 Knapsack Problem using OpenMP.
 * 
 * Uses best-first search with a thread-safe priority queue to explore promising nodes first.
 * Multiple worker threads extract nodes, calculate bounds, generate children, and update
 * the global best solution concurrently.
 * 
 * Algorithm steps:
 * 1. Sort items by value/weight ratio (greedy heuristic)
 * 2. Initialize shared queue with root node (empty knapsack)
 * 3. Launch worker threads that:
 *    - Extract most promising node (highest bound)
 *    - Generate children: include next item (if feasible) and exclude next item
 *    - Prune branches with bound <= current best
 *    - Update global best solution atomically
 * 4. Continue until queue is empty and all threads are idle
 * 
 * @param items Array of available items
 * @param n Number of items
 * @param capacity Maximum knapsack capacity
 * @param max_value Output: optimal value achieved
 * @param best_items Output: array of selected items
 * @param best_count Output: number of selected items
 * @param num_threads Number of worker threads to use
 * @return Root of the search tree (for solution reconstruction)
 */
TreeNode* branch_and_bound_parallel(Item* items, int n, float capacity, 
                                   float* max_value, Item** best_items, 
                                   int* best_count, int num_threads) {
    // Shallow tasking cutoff to prevent creating tiny tasks deep in the tree
    const int TASK_CUTOFF_LEVEL = 12;
    
    // Sort items by value/weight ratio (descending)
    // This improves bound quality and pruning effectiveness
    std::sort(items, items + n, compare_items);
    print_sorted_items(items, n);
    
    // Global best solution (shared across threads)
    float global_best_value = 0.0f;
    TreeNode* global_best_node = nullptr;
    omp_lock_t best_lock;
    omp_init_lock(&best_lock);
    
    // Initialize root node representing empty knapsack
    TreeNode* root = new TreeNode();
    root->level = -1;
    root->t_value = 0.0f;
    root->t_weight = 0.0f;
    root->bound = calculate_bound(root, items, n, capacity);
    
    global_best_node = root;
    
    std::atomic<int> nodes_explored(0);
    std::atomic<int> nodes_pruned(0);
    
    // Recursive task-based explorer
    std::function<void(TreeNode*)> explore;
    explore = [&](TreeNode* current) {
        nodes_explored.fetch_add(1, std::memory_order_relaxed);
        
        float current_best;
        #pragma omp critical(read_best)
        {
            current_best = global_best_value;
        }
        
        // Prune hopeless branches
        if (current->bound <= current_best) {
            nodes_pruned.fetch_add(1, std::memory_order_relaxed);
            return;
        }
        
        // Leaf: all items considered
        if (current->level == n - 1) {
            return;
        }
        
        int next_level = current->level + 1;
        
        // Left child: include next item if feasible
        if (current->t_weight + items[next_level].weight <= capacity) {
            TreeNode* left_child = current->add(
                true,
                items[next_level],
                current->t_weight + items[next_level].weight,
                current->t_value + items[next_level].value,
                next_level
            );
            
            left_child->bound = calculate_bound(left_child, items, n, capacity);
            
            // Update best value if improved
            if (left_child->t_value > current_best) {
                omp_set_lock(&best_lock);
                if (left_child->t_value > global_best_value) {
                    global_best_value = left_child->t_value;
                    global_best_node = left_child;
                    current_best = global_best_value;
                }
                omp_unset_lock(&best_lock);
            }
            
            // Spawn task (or recurse inline) if promising
            if (left_child->bound > current_best) {
                if (next_level < TASK_CUTOFF_LEVEL) {
                    #pragma omp task firstprivate(left_child) shared(explore)
                    {
                        explore(left_child);
                    }
                } else {
                    explore(left_child);
                }
            }
        }
        
        // Right child: exclude next item (always feasible)
        TreeNode* right_child = current->add(
            false,
            items[next_level],
            current->t_weight,
            current->t_value,
            next_level
        );
        
        right_child->bound = calculate_bound(right_child, items, n, capacity);
        
        #pragma omp critical(read_best)
        {
            current_best = global_best_value;
        }
        
        if (right_child->bound > current_best) {
            if (next_level < TASK_CUTOFF_LEVEL) {
                #pragma omp task firstprivate(right_child) shared(explore)
                {
                    explore(right_child);
                }
            } else {
                explore(right_child);
            }
        }
    };
    
    // Parallel region with a single initial task; implicit barrier waits for all tasks
    omp_set_num_threads(num_threads);
    #pragma omp parallel
    {
        #pragma omp single nowait
        {
            explore(root);
        }
    }
    
    omp_destroy_lock(&best_lock);
    
    print_statistics(nodes_explored.load(), nodes_pruned.load());
    
    // Reconstruct the solution by tracing path from root to best_node
    *max_value = global_best_value;
    *best_items = new Item[n];
    *best_count = 0;
    reconstruct_solution(root, global_best_node, best_items, best_count);
    
    return root;
}
