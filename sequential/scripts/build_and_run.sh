#!/bin/bash
mkdir -p out
g++ -std=c++11 -I../include -Iinclude \
    src/index.cpp \
    src/branch_and_bound.cpp \
    ../src/knapsack_utils.cpp \
    ../src/output_display.cpp \
    ../src/parser/parser.cpp \
    -o out/index
time ./out/index
rm -rf out
