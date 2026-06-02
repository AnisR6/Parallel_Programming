#!/usr/bin/env bash
# run_ex2.sh – compile and run Exercise 2 loop optimization benchmarks
# Usage: bash run_ex2.sh

set -e

echo "=== Compiling without optimisation (-O0) ==="
gcc -O0 -o ex2_O0 ex2_bench.c -lm
echo "  -> ex2_O0"

echo "=== Compiling with full optimisation (-O3) ==="
gcc -O3 -o ex2_O3 ex2_bench.c -lm
echo "  -> ex2_O3"

echo ""
echo "=========================================="
echo "  Results: -O0 (no optimisation)"
echo "=========================================="
./ex2_O0

echo ""
echo "=========================================="
echo "  Results: -O3 (full optimisation)"
echo "=========================================="
./ex2_O3

echo ""
echo "=========================================="
echo "  perf stat on -O3 binary"
echo "=========================================="
perf stat -e cycles,instructions,cache-misses,cache-references ./ex2_O3
