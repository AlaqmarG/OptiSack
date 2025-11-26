#!/bin/bash
# Run comprehensive benchmarks with CSV output for analysis
# Tests multiple datasets and thread counts

if [ ! -f "out/benchmark_parallel" ]; then
    echo "Error: benchmark_parallel not found. Run build_benchmark.sh first."
    exit 1
fi

echo "======================================================================"
echo "Running Full Parallel Benchmarks (50 iterations each)"
echo "======================================================================"
echo ""

# Create results directory
mkdir -p results
RESULTS_FILE="results/parallel_benchmarks.csv"

# Write CSV header
echo "dataset,items,threads,avg_time_s,total_time_s,optimal_value,speedup_vs_seq" > $RESULTS_FILE

# Thread counts to test
THREAD_COUNTS="${@:-1 2 4 8}"

# Datasets to benchmark
datasets=(
    "fast:benchmark_fast_85items.txt:Fast Baseline"
    "medium:benchmark_medium_100items.txt:Medium Difficulty"
    "medium_hard:benchmark_medium_hard_112items.txt:Medium-Hard"
    "very_hard:benchmark_very_hard_110items.txt:Very Hard"
    "extreme:benchmark_extreme_121items.txt:Extreme"
)

for dataset_info in "${datasets[@]}"; do
    IFS=':' read -r prefix filename description <<< "$dataset_info"
    
    DATASET_FILE="../sequential/data/$filename"
    
    if [ ! -f "$DATASET_FILE" ]; then
        echo "Warning: Dataset file not found: $DATASET_FILE, skipping..."
        continue
    fi
    
    echo "======================================================================"
    echo "Benchmarking: $description"
    echo "File: $filename"
    echo "======================================================================"
    
    # Update test_config.h to point to this dataset
    cat > include/test_config.h << EOF
#ifndef TEST_CONFIG_H
#define TEST_CONFIG_H
#define TEST_FILE "data/$filename"
#endif
EOF
    
    # Get item count from file (first line)
    items=$(head -n 1 "$DATASET_FILE" 2>/dev/null || echo "0")
    
    # Run benchmark for each thread count
    for threads in $THREAD_COUNTS; do
        echo "--- Testing with $threads thread(s) ---"
        
        # Run benchmark and capture output
        ./out/benchmark_parallel $threads > out/temp_benchmark_output.txt 2>&1
        
        # Extract results
        avg_time=$(grep "Average time per run:" out/temp_benchmark_output.txt | grep -oE "[0-9]+\.[0-9]+" | head -1)
        total_time=$(grep "Total time.*runs" out/temp_benchmark_output.txt | grep -oE "[0-9]+\.[0-9]+" | head -1)
        optimal=$(grep "Optimal value:" out/temp_benchmark_output.txt | grep -oE "[0-9]+\.[0-9]+" | head -1)
        
        # Show summary
        grep -A 10 "^====.*RESULTS" out/temp_benchmark_output.txt || tail -10 out/temp_benchmark_output.txt
        
        # Write to CSV (speedup calculation would need sequential baseline)
        if [ -n "$avg_time" ] && [ -n "$total_time" ] && [ -n "$optimal" ]; then
            echo "$filename,$items,$threads,$avg_time,$total_time,$optimal," >> $RESULTS_FILE
        fi
        
        echo ""
    done
    
    echo ""
done

rm -f out/temp_benchmark_output.txt

echo "======================================================================"
echo "All Benchmarks Complete!"
echo "======================================================================"
echo ""
echo "Results summary:"
if [ -f "$RESULTS_FILE" ]; then
    cat $RESULTS_FILE | column -t -s ','
fi


