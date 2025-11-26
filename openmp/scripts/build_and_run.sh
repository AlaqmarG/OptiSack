#!/bin/bash

# Build and run the main parallel program
# Usage: ./build_and_run.sh [num_threads]

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OPENMP_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

cd "$OPENMP_DIR"

echo "Building parallel branch and bound..."
mkdir -p out

# Detect compiler and set appropriate OpenMP flags
# On macOS, check if g++ is actually GCC or Apple's clang
if command -v g++ &> /dev/null; then
    # Check if g++ is actually GCC (not Apple clang)
    if g++ --version 2>&1 | grep -q "Free Software Foundation" || g++ --version 2>&1 | grep -q "GCC"; then
        COMPILER="g++"
        OPENMP_FLAGS="-fopenmp"
    else
        # g++ is actually clang++ (common on macOS)
        COMPILER="g++"
        if [[ "$OSTYPE" == "darwin"* ]] || [[ "$(uname)" == "Darwin" ]]; then
            LIBOMP_PREFIX=$(brew --prefix libomp 2>/dev/null || echo "/opt/homebrew/opt/libomp")
            OPENMP_FLAGS="-Xpreprocessor -fopenmp -I${LIBOMP_PREFIX}/include -L${LIBOMP_PREFIX}/lib -lomp"
        else
            OPENMP_FLAGS="-fopenmp"
        fi
    fi
elif command -v clang++ &> /dev/null; then
    COMPILER="clang++"
    if [[ "$OSTYPE" == "darwin"* ]] || [[ "$(uname)" == "Darwin" ]]; then
        # macOS with Homebrew libomp
        LIBOMP_PREFIX=$(brew --prefix libomp 2>/dev/null || echo "/opt/homebrew/opt/libomp")
        OPENMP_FLAGS="-Xpreprocessor -fopenmp -I${LIBOMP_PREFIX}/include -L${LIBOMP_PREFIX}/lib -lomp"
    else
        OPENMP_FLAGS="-fopenmp"
    fi
else
    echo "Error: No suitable compiler found (g++ or clang++)"
    exit 1
fi

echo "Using compiler: $COMPILER"
echo "OpenMP flags: $OPENMP_FLAGS"

$COMPILER -std=c++11 -I../include -Iinclude $OPENMP_FLAGS \
    src/index.cpp \
    src/branch_and_bound_parallel.cpp \
    ../src/knapsack_utils.cpp \
    ../src/output_display.cpp \
    ../src/parser/parser.cpp \
    -o out/index

if [ $? -eq 0 ]; then
    echo "✓ Build successful!"
    echo ""
    echo "Running program..."
    echo ""
    # Run from openmp directory so data symlink works
    ./out/index "${1:-4}"
else
    echo "✗ Build failed!"
    exit 1
fi
