#!/bin/bash
#SBATCH --job-name=ex3_intrinsics
#SBATCH --output=ex3_intrinsics_%j.out
#SBATCH --error=ex3_intrinsics_%j.err
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --time=00:10:00

# ── Compile ────────────────────────────────────────────────────────────────────
# -O1           : same optimisation level as Ex1 / Ex2 for fair comparison
# -fopenmp      : needed for omp_get_wtime()
# -msse2        : enable SSE2 so xmmintrin.h intrinsics are available
# NO -ftree-vectorize, NO -march=native auto-vectorisation
gcc -O1 -fopenmp -msse2 ex3_intrinsics.c -o vector_intrinsics

echo "=== Exercise 3 – SSE Intrinsics (float) ==="
echo ""

SIZES="128 256 512 1024 2048"

for SIZE in $SIZES; do
    echo "--- size = $SIZE ---"
    ./vector_intrinsics $SIZE
    echo ""
done

# ── perf verification (packed-float event r1010) ───────────────────────────────

echo "=== perf stat – packed float ops (size 2048) ==="
perf stat -e r1010 ./vector_intrinsics 2048 2>&1

echo ""
echo "Done."
