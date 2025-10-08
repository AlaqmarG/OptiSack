/**
 * Dataset Generator for 0/1 Knapsack Problem
 * Generates test instances with configurable parameters and distributions
 * 
 * Usage: ./generate_dataset <num_items> <distribution_type> <capacity_percentage> <output_file>
 * 
 * Distribution types:
 *   uncorrelated      - Random values and weights (easiest for pruning)
 *   weakly_correlated - Value ≈ weight + noise
 *   strongly_correlated - Value = weight + constant
 *   subset_sum        - Value = weight (hardest for branch & bound)
 *   inverse_strong    - High value = low weight (moderate difficulty)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

// Random float between min and max
float rand_range(float min, float max) {
    return min + (float)rand() / RAND_MAX * (max - min);
}

// Random integer between min and max (inclusive)
int rand_int(int min, int max) {
    return min + rand() % (max - min + 1);
}

void generate_dataset(int num_items, const char* distribution, float capacity_pct, const char* output_file) {
    FILE* file = fopen(output_file, "w");
    if (!file) {
        fprintf(stderr, "Error: Could not create output file %s\n", output_file);
        exit(1);
    }
    
    // Seed random number generator
    srand(time(NULL) + rand());
    
    // Write number of items
    fprintf(file, "%d\n", num_items);
    
    float total_weight = 0.0f;
    
    // Generate items based on distribution type
    if (strcmp(distribution, "uncorrelated") == 0) {
        // Uncorrelated: completely random values and weights
        for (int i = 0; i < num_items; i++) {
            float weight = rand_range(1.0f, 100.0f);
            float value = rand_range(1.0f, 100.0f);
            fprintf(file, "%d %.2f %.2f\n", i, value, weight);
            total_weight += weight;
        }
        
    } else if (strcmp(distribution, "weakly_correlated") == 0) {
        // Weakly correlated: value ≈ weight + noise
        for (int i = 0; i < num_items; i++) {
            float weight = rand_range(1.0f, 100.0f);
            float value = weight + rand_range(-10.0f, 10.0f);
            if (value < 1.0f) value = 1.0f; // Keep positive
            fprintf(file, "%d %.2f %.2f\n", i, value, weight);
            total_weight += weight;
        }
        
    } else if (strcmp(distribution, "strongly_correlated") == 0) {
        // Strongly correlated: value = weight + constant
        float constant = 10.0f;
        for (int i = 0; i < num_items; i++) {
            float weight = rand_range(1.0f, 100.0f);
            float value = weight + constant;
            fprintf(file, "%d %.2f %.2f\n", i, value, weight);
            total_weight += weight;
        }
        
    } else if (strcmp(distribution, "subset_sum") == 0) {
        // Subset sum: value = weight (hardest for branch & bound)
        for (int i = 0; i < num_items; i++) {
            float weight = rand_range(1.0f, 100.0f);
            float value = weight; // Same as weight
            fprintf(file, "%d %.2f %.2f\n", i, value, weight);
            total_weight += weight;
        }
        
    } else if (strcmp(distribution, "inverse_strong") == 0) {
        // Inverse strongly correlated: high value = low weight
        for (int i = 0; i < num_items; i++) {
            float weight = rand_range(1.0f, 100.0f);
            float value = 100.0f - weight + rand_range(10.0f, 30.0f);
            fprintf(file, "%d %.2f %.2f\n", i, value, weight);
            total_weight += weight;
        }
        
    } else {
        fprintf(stderr, "Error: Unknown distribution type '%s'\n", distribution);
        fprintf(stderr, "Valid types: uncorrelated, weakly_correlated, strongly_correlated, subset_sum, inverse_strong\n");
        fclose(file);
        exit(1);
    }
    
    // Calculate capacity based on percentage of total weight
    float capacity = total_weight * (capacity_pct / 100.0f);
    fprintf(file, "%.2f\n", capacity);
    
    fclose(file);
    
    printf("Generated dataset: %s\n", output_file);
    printf("  Items: %d\n", num_items);
    printf("  Distribution: %s\n", distribution);
    printf("  Total weight: %.2f\n", total_weight);
    printf("  Capacity: %.2f (%.0f%% of total)\n", capacity, capacity_pct);
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        printf("Usage: %s <num_items> <distribution> <capacity_pct> <output_file>\n", argv[0]);
        printf("\nDistribution types:\n");
        printf("  uncorrelated       - Random values and weights\n");
        printf("  weakly_correlated  - Value ≈ weight + noise\n");
        printf("  strongly_correlated- Value = weight + constant\n");
        printf("  subset_sum         - Value = weight (hardest)\n");
        printf("  inverse_strong     - High value = low weight\n");
        printf("\nCapacity percentage: 0-100 (e.g., 50 for 50%% of total weight)\n");
        printf("\nExample: %s 500 subset_sum 50 data/hard_500.txt\n", argv[0]);
        return 1;
    }
    
    int num_items = atoi(argv[1]);
    const char* distribution = argv[2];
    float capacity_pct = atof(argv[3]);
    const char* output_file = argv[4];
    
    if (num_items <= 0) {
        fprintf(stderr, "Error: Number of items must be positive\n");
        return 1;
    }
    
    if (capacity_pct <= 0 || capacity_pct > 100) {
        fprintf(stderr, "Error: Capacity percentage must be between 0 and 100\n");
        return 1;
    }
    
    generate_dataset(num_items, distribution, capacity_pct, output_file);
    
    return 0;
}

