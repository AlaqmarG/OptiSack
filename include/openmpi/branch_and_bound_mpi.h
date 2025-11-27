#ifndef BRANCH_AND_BOUND_MPI_H
#define BRANCH_AND_BOUND_MPI_H

#include <mpi.h>
#include "parser/parser.h"

struct MpiStats {
    int nodes_explored;
    int nodes_pruned;
};

void branch_and_bound_mpi(Item* items,
                          int item_count,
                          float capacity,
                          float* global_best_value,
                          Item** best_items,
                          int* best_count,
                          MpiStats* stats,
                          MPI_Comm comm);

#endif // BRANCH_AND_BOUND_MPI_H
