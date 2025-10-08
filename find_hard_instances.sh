#!/bin/bash
# Automated script to find challenging knapsack instances
# Tests progressively harder configurations until finding ones in the 5-30 second range

echo "======================================================================"
echo "Searching for Hard Knapsack Problem Instances"
echo "======================================================================"
echo ""
echo "Target: Find instances with 5-30 second execution times"
echo "Strategy: Test subset-sum distributions with increasing item counts"
echo ""

# Create results directory
mkdir -p results
RESULTS_FILE="results/timing_results.csv"

# Write CSV header
echo "filename,items,capacity,time_seconds,optimal_value,items_selected" > $RESULTS_FILE

# Test configurations: item_count, distribution, capacity_pct
# Start with subset_sum (hardest) and progressively increase size
test_configs=(
    "85:subset_sum:50"
    "90:subset_sum:50"
    "95:subset_sum:50"
    "100:subset_sum:50"
    "105:subset_sum:50"
    "110:subset_sum:50"
    "115:subset_sum:50"
    "120:subset_sum:50"
)

echo "Phase 1: Testing configurations to find good range..."
echo ""

for config in "${test_configs[@]}"; do
    IFS=':' read -r items dist cap <<< "$config"
    
    filename="data/test_${items}_${dist}_${cap}pct.txt"
    
    # Generate dataset
    ./generate_dataset $items $dist $cap $filename > /dev/null 2>&1
    
    if [ ! -f "$filename" ]; then
        echo "Error: Failed to generate $filename"
        continue
    fi
    
    # Quick test
    echo "Testing: $items items, $dist, ${cap}% capacity..."
    
    # Run timing test with timeout of 60 seconds
    timeout 60s ./test_timing "$filename" > temp_output.txt 2>&1
    exit_code=$?
    
    if [ $exit_code -eq 124 ]; then
        echo "  ⚠️  TIMEOUT (>60s) - Too hard, skipping"
        echo "$filename,$items,N/A,>60,N/A,N/A" >> $RESULTS_FILE
        rm "$filename"  # Remove file that's too hard
        continue
    fi
    
    # Extract timing
    time_line=$(grep "^CSV:" temp_output.txt)
    if [ -z "$time_line" ]; then
        echo "  ❌ Error running test"
        continue
    fi
    
    # Parse CSV output
    IFS=',' read -r csv_file csv_items csv_cap csv_time csv_value csv_selected <<< "$time_line"
    csv_time=$(echo "$csv_time" | xargs)  # Trim whitespace
    
    # Add to results
    echo "$filename,$csv_items,$csv_cap,$csv_time,$csv_value,$csv_selected" >> $RESULTS_FILE
    
    # Print result with color coding
    time_ms=$(echo "$csv_time * 1000" | bc)
    if (( $(echo "$csv_time < 1.0" | bc -l) )); then
        echo "  ⚡ Fast: ${csv_time}s (${time_ms} ms)"
    elif (( $(echo "$csv_time < 5.0" | bc -l) )); then
        echo "  ✓ Good: ${csv_time}s"
    elif (( $(echo "$csv_time < 30.0" | bc -l) )); then
        echo "  ✅ EXCELLENT: ${csv_time}s - KEEPER!"
    else
        echo "  🎯 Very Hard: ${csv_time}s - KEEPER!"
    fi
    
    echo ""
done

rm -f temp_output.txt

echo "======================================================================"
echo "Phase 1 Complete!"
echo "======================================================================"
echo ""
echo "Results saved to: $RESULTS_FILE"
echo ""

# Analyze results and show summary
echo "Summary of Results:"
echo "-------------------"
cat $RESULTS_FILE | column -t -s ','

echo ""
echo "======================================================================"
echo "Recommended datasets for benchmarking:"
echo "======================================================================"

# Find files in different time ranges
echo ""
echo "Fast baseline (< 1s):"
awk -F',' 'NR>1 && $4 > 0 && $4 < 1 {print "  " $1 " - " $4 "s"}' $RESULTS_FILE | head -1

echo ""
echo "Medium difficulty (1-5s):"
awk -F',' 'NR>1 && $4 >= 1 && $4 < 5 {print "  " $1 " - " $4 "s"}' $RESULTS_FILE | head -2

echo ""
echo "Hard (5-15s):"
awk -F',' 'NR>1 && $4 >= 5 && $4 < 15 {print "  " $1 " - " $4 "s"}' $RESULTS_FILE | head -2

echo ""
echo "Very Hard (15-30s):"
awk -F',' 'NR>1 && $4 >= 15 && $4 < 30 {print "  " $1 " - " $4 "s"}' $RESULTS_FILE | head -2

echo ""
echo "======================================================================"

