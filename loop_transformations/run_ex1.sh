#!/usr/bin/env bash
# run_ex1.sh – compile and run the Exercise 1 strength-reduction benchmarks
# Usage: bash run_ex1.sh

set -e

echo "=== Compiling without optimisation (-O0) ==="
gcc -O0 -o exo1_O0 ex1_bench.c
echo "  -> exo1_O0"

echo "=== Compiling with full optimisation (-O3) ==="
gcc -O3 -o exo1_O3 ex1_bench.c
echo "  -> exo1_O3"

echo ""
echo "=========================================="
echo "  Results: -O0 (no optimisation)"
echo "=========================================="
./exo1_O0

echo ""
echo "=========================================="
echo "  Results: -O3 (full optimisation)"
echo "=========================================="
./exo1_O3

echo ""
echo "=========================================="
echo "  Assembly comparison (godbolt-style)"
echo "  Dump -O3 asm to exo1_O3.s"
echo "=========================================="
gcc -O3 -S -o exo1_O3.s ex1_bench.c
echo "  -> exo1_O3.s  (open with: less exo1_O3.s)"

echo ""
echo "=========================================="
echo "  perf stat on -O3 binary"
echo "=========================================="
perf stat -e cycles,instructions,cache-misses,cache-references ./exo1_O3
