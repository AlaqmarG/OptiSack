mkdir -p out
g++ -std=c++11 index.cpp branch_and_bound.cpp knapsack_utils.cpp output_display.cpp parser/parser.cpp -o out/index
time ./out/index
rm -rf out