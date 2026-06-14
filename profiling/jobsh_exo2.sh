#!/bin/bash

#SBATCH --job-name=mg_benchmark
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=12
#SBATCH --exclusive

echo "============================================"
echo "Compiling..."
echo "============================================"

gcc -O3 -std=c99 real.c c_timers.c print_results.c randdp.c wtime.c -o real_seq -lm
gcc -O3 -fopenmp -std=c99 real_omp.c c_timers.c print_results.c randdp.c wtime.c -o real_omp -lm

echo "Done."
echo ""

echo "============================================"
echo "Running Sequential"
echo "============================================"
time ./real_seq

echo ""
echo "============================================"
echo "Running OpenMP - 1 thread"
echo "============================================"
export OMP_NUM_THREADS=1
time ./real_omp

echo ""
echo "============================================"
echo "Running OpenMP - 2 threads"
echo "============================================"
export OMP_NUM_THREADS=2
time ./real_omp

echo ""
echo "============================================"
echo "Running OpenMP - 6 threads"
echo "============================================"
export OMP_NUM_THREADS=6
time ./real_omp

echo ""
echo "============================================"
echo "Running OpenMP - 12 threads"
echo "============================================"
export OMP_NUM_THREADS=12
time ./real_omp

echo ""
echo "============================================"
echo "Done."
echo "============================================"