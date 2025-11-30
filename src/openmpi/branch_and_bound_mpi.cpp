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
#include "branch_and_bound.h"  // For CompareNode comparator

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

float local_best_value = 0.0f;
TreeNode* best_node = nullptr;
TreeNode* best_root = nullptr;
std::atomic<long long> local_nodes_explored(0);
std::atomic<long long> local_nodes_pruned(0);

// Helper: best-first search using a local priority queue on this rank,
// starting from the given node within the provided tree root.
auto explore_with_pq = [&](TreeNode* root, TreeNode* start_node) {
    std::priority_queue<TreeNode*, std::vector<TreeNode*>, CompareNode> pq;
    pq.push(start_node);

    while (!pq.empty()) {
        TreeNode* current = pq.top();
        pq.pop();

        local_nodes_explored.fetch_add(1, std::memory_order_relaxed);

        if (current->bound <= local_best_value) {
            local_nodes_pruned.fetch_add(1, std::memory_order_relaxed);
            continue;
        }

        if (current->level == item_count - 1) {
            continue;
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
                best_root = root;
            }

            if (left_child->bound > local_best_value) {
                pq.push(left_child);
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
            pq.push(right_child);
        }
    }
};

    // Partition the search tree across ranks using fixed decision prefixes.
    // We take the first prefix_bits items and assign each of the 2^prefix_bits
    // include/exclude patterns round-robin to MPI ranks.
    // Using a small prefix (2) limits duplicate work and keeps pruning effective.
    int prefix_bits = std::min(2, item_count);  // up to 4 disjoint prefixes
    int total_patterns = 1 << prefix_bits;

    for (int pattern = 0; pattern < total_patterns; ++pattern) {
        if (pattern % world_size != world_rank) {
            // This rank skips patterns assigned to other ranks, but still
            // participates in the global best-value synchronization below.
            // (local_best_value remains whatever it was from previous work.)
        } else {

            // Fresh tree for this pattern
            TreeNode* root = new TreeNode();
            root->level = -1;
            root->t_value = 0.0f;
            root->t_weight = 0.0f;
            root->bound = calculate_bound(root, items, item_count, capacity);

            TreeNode* start_node = root;
            bool pruned_prefix = false;

            // Apply prefix decisions according to bits of 'pattern'
            for (int lvl = 0; lvl < prefix_bits; ++lvl) {
                int item_index = lvl;
                bool include = (pattern & (1 << lvl)) != 0;

                if (include && start_node->t_weight + items[item_index].weight <= capacity) {
                    start_node = start_node->add(
                        true, items[item_index],
                        start_node->t_weight + items[item_index].weight,
                        start_node->t_value + items[item_index].value,
                        item_index
                    );
                } else {
                    // Either exclude by choice or because including would overflow
                    start_node = start_node->add(
                        false, items[item_index],
                        start_node->t_weight,
                        start_node->t_value,
                        item_index
                    );
                }
                start_node->bound = calculate_bound(start_node, items, item_count, capacity);

                if (start_node->bound <= local_best_value) {
                    // Entire subtree under this prefix cannot beat current best for this rank
                    pruned_prefix = true;
                    break;
                }
            }

            if (pruned_prefix) {
                delete root;
            } else {
                double prev_best = local_best_value;
                explore_with_pq(root, start_node);

                if (local_best_value <= prev_best) {
                    // This pattern did not improve the local best; discard its tree
                    delete root;
                } else {
                    // Keep only the tree containing the current best solution
                    // (any previous best_root can be safely deleted now)
                    if (best_root && best_root != root) {
                        delete best_root;
                    }
                    best_root = root;
                }
            }
        }

        // After each pattern, synchronize the best value across ranks so that
        // later prefixes benefit from better global incumbents and prune more.
        float synced_best = 0.0f;
        MPI_Allreduce(&local_best_value, &synced_best, 1, MPI_FLOAT, MPI_MAX, comm);
        local_best_value = synced_best;
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

    if (world_rank == winning_rank && best_root && best_node) {
        reconstruct_solution(best_root, best_node, solution_items);
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
