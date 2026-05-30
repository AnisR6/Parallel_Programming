#!/bin/bash

#SBATCH --job-name=ex2_openmp_simd
#SBATCH --output=ex2_openmp_simd_%j.log
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --mem=1G
#SBATCH --time=00:30:00
#SBATCH --partition=lva

module load gcc/12.2.0-gcc-8.5.0-p4pe45v

echo "=== Compiling ==="

gcc -O1 -fopenmp autovectorize.c -o vector_seq
echo "Compiled: vector_seq  (sequential baseline)"

gcc -O1 -fopenmp ex2_float.c -o vector_omp_float
echo "Compiled: vector_omp_float  (OpenMP SIMD, float)"

gcc -O1 -fopenmp ex2_double.c -o vector_omp_double
echo "Compiled: vector_omp_double  (OpenMP SIMD, double)"

echo ""

SIZES=(128 256 512 1024 2048)

echo "=== Sequential Baseline (float) ==="
for SIZE in "${SIZES[@]}"; do
    echo -n "Size $SIZE: "
    ./vector_seq $SIZE
done

echo ""
echo "=== OpenMP SIMD (float) ==="
for SIZE in "${SIZES[@]}"; do
    echo -n "Size $SIZE: "
    ./vector_omp_float $SIZE
done

echo ""
echo "=== OpenMP SIMD (double) ==="
for SIZE in "${SIZES[@]}"; do
    echo -n "Size $SIZE: "
    ./vector_omp_double $SIZE
done

echo ""
echo "=== perf: SSE_FP_PACKED (r1010) — OMP float ==="
perf stat -e r1010 ./vector_omp_float 2048

echo ""
echo "=== perf: SSE_DOUBLE_PRECISION (r8010) — OMP double ==="
perf stat -e r8010 ./vector_omp_double 2048

echo ""
echo "=== Done ==="
