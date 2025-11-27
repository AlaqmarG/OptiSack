/**
 * MPI Branch and Bound implementation for 0/1 Knapsack
 *
 * Uses MPI for distributed computing with sequential execution on rank 0
 * and result broadcasting to all processes.
 */

#include <mpi.h>
#include <algorithm>
#include <queue>
#include <vector>
#include <functional>
#include <atomic>
#include "branch_and_bound_mpi.h"
#include "knapsack_utils.h"
#include "output_display.h"
#include "tree_node.h"

namespace {

void reconstruct_solution(TreeNode* root, TreeNode* best_node,
                         std::vector<Item>& solution_items) {
    std::function<bool(TreeNode*, float, float, std::vector<Item>&)> find_path;
    
    find_path = [&](TreeNode* node, float target_value, float target_weight,
                     std::vector<Item>& path) -> bool {
        if (!node) return false;
        
        if (node->t_value == target_value && node->t_weight == target_weight) {
            return true;
        }
        
        if (node->left) {
            if (node->left->included) {
                path.push_back(node->left->item);
            }
            if (find_path(node->left, target_value, target_weight, path)) {
                return true;
            }
            if (node->left->included) {
                path.pop_back();
            }
        }
        
        if (node->right) {
            if (find_path(node->right, target_value, target_weight, path)) {
                return true;
            }
        }
        
        return false;
    };
    
    find_path(root, best_node->t_value, best_node->t_weight, solution_items);
}

} // namespace

void branch_and_bound_mpi(Item* items,
                          int item_count,
                          float capacity,
                          float* global_best_value,
                          Item** best_items,
                          int* best_count,
                          MpiStats* stats,
                          MPI_Comm comm) {
    int world_rank = 0;
    int world_size = 1;
    MPI_Comm_rank(comm, &world_rank);
    MPI_Comm_size(comm, &world_size);

    if (item_count == 0) {
        if (global_best_value) *global_best_value = 0.0f;
        if (best_items) *best_items = nullptr;
        if (best_count) *best_count = 0;
        if (stats) {
            stats->nodes_explored = 0;
            stats->nodes_pruned = 0;
        }
        return;
    }

    // Sort items
    std::sort(items, items + item_count, compare_items);
    if (world_rank == 0) {
        print_sorted_items(items, item_count);
    }

    // Use parallel branch and bound across MPI ranks
    const int TASK_CUTOFF_LEVEL = 12;

    float local_best_value = 0.0f;
    TreeNode* root = new TreeNode();
    root->level = -1;
    root->t_value = 0.0f;
    root->t_weight = 0.0f;
    root->bound = calculate_bound(root, items, item_count, capacity);

    TreeNode* best_node = root;
    std::atomic<long long> local_nodes_explored(0);
    std::atomic<long long> local_nodes_pruned(0);

    // Each rank explores a portion of the search space
    std::function<void(TreeNode*)> explore;
    explore = [&](TreeNode* current) {
        local_nodes_explored.fetch_add(1, std::memory_order_relaxed);

        if (current->bound <= local_best_value) {
            local_nodes_pruned.fetch_add(1, std::memory_order_relaxed);
            return;
        }

        if (current->level == item_count - 1) {
            return;
        }

        int next_level = current->level + 1;

        // Left child (include item)
        if (current->t_weight + items[next_level].weight <= capacity) {
            TreeNode* left_child = current->add(
                true, items[next_level],
                current->t_weight + items[next_level].weight,
                current->t_value + items[next_level].value,
                next_level
            );

            left_child->bound = calculate_bound(left_child, items, item_count, capacity);

            if (left_child->t_value > local_best_value) {
                local_best_value = left_child->t_value;
                best_node = left_child;
            }

            if (left_child->bound > local_best_value) {
                explore(left_child);
            }
        }

        // Right child (exclude item)
        TreeNode* right_child = current->add(
            false, items[next_level],
            current->t_weight, current->t_value,
            next_level
        );

        right_child->bound = calculate_bound(right_child, items, item_count, capacity);

        if (right_child->bound > local_best_value) {
            explore(right_child);
        }
    };

    // Divide initial work across ranks by starting from different root decisions
    // This is a simple load balancing approach for demonstration
    if (world_rank == 0) {
        // Rank 0 starts with root (explores all possibilities)
        explore(root);
    } else {
        // Other ranks start with partial solutions to distribute work
        // This is a simplified approach - in practice, you'd want better load balancing
        int start_decisions = world_rank % 4;  // Simple distribution

        TreeNode* start_node = root;
        for (int i = 0; i < start_decisions && i < item_count; ++i) {
            if (start_node->t_weight + items[i].weight <= capacity) {
                start_node = start_node->add(
                    true, items[i],
                    start_node->t_weight + items[i].weight,
                    start_node->t_value + items[i].value,
                    i
                );
                start_node->bound = calculate_bound(start_node, items, item_count, capacity);
            } else {
                break;
            }
        }
        explore(start_node);
    }

    // Find global best across all ranks
    float global_best = 0.0f;
    MPI_Allreduce(&local_best_value, &global_best, 1, MPI_FLOAT, MPI_MAX, comm);

    // Gather statistics
    long long total_nodes_explored = 0;
    long long total_nodes_pruned = 0;
    MPI_Allreduce(&local_nodes_explored, &total_nodes_explored, 1, MPI_LONG_LONG, MPI_SUM, comm);
    MPI_Allreduce(&local_nodes_pruned, &total_nodes_pruned, 1, MPI_LONG_LONG, MPI_SUM, comm);

    // Find which rank has the global best solution
    struct {
        float value;
        int rank;
    } local_result = {local_best_value, world_rank}, global_result;

    MPI_Allreduce(&local_result, &global_result, 1, MPI_FLOAT_INT, MPI_MAXLOC, comm);
    int winning_rank = global_result.rank;

    // The winning rank reconstructs and broadcasts the solution
    std::vector<Item> solution_items;
    int solution_count = 0;

    if (world_rank == winning_rank) {
        reconstruct_solution(root, best_node, solution_items);
        solution_count = solution_items.size();
    }

    // Broadcast solution count and items from winning rank
    MPI_Bcast(&solution_count, 1, MPI_INT, winning_rank, comm);

    if (world_rank != winning_rank) {
        solution_items.resize(solution_count);
    }

    if (solution_count > 0) {
        MPI_Datatype MPI_ITEM;
        int block_lengths[3] = {1, 1, 1};
        MPI_Aint offsets[3];
        MPI_Datatype types[3] = {MPI_INT, MPI_FLOAT, MPI_FLOAT};

        Item item_sample;
        MPI_Aint base_address;
        MPI_Get_address(&item_sample, &base_address);
        MPI_Get_address(&item_sample.id, &offsets[0]);
        MPI_Get_address(&item_sample.value, &offsets[1]);
        MPI_Get_address(&item_sample.weight, &offsets[2]);

        offsets[0] = MPI_Aint_diff(offsets[0], base_address);
        offsets[1] = MPI_Aint_diff(offsets[1], base_address);
        offsets[2] = MPI_Aint_diff(offsets[2], base_address);

        MPI_Type_create_struct(3, block_lengths, offsets, types, &MPI_ITEM);
        MPI_Type_commit(&MPI_ITEM);

        MPI_Bcast(solution_items.data(), solution_count, MPI_ITEM, winning_rank, comm);

        MPI_Type_free(&MPI_ITEM);
    }    // Set output parameters
    if (global_best_value) {
        *global_best_value = global_best;
    }
    if (best_count) {
        *best_count = solution_count;
    }
    if (best_items) {
        *best_items = new Item[solution_count];
        for (int i = 0; i < solution_count; ++i) {
            (*best_items)[i] = solution_items[i];
        }
    }
    if (stats) {
        stats->nodes_explored = (int)total_nodes_explored;
        stats->nodes_pruned = (int)total_nodes_pruned;
    }
}
