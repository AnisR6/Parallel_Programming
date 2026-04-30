#!/bin/bash
#SBATCH --job-name=mandelbrot_omp
#SBATCH --output=mandelbrot_omp_%j.out
#SBATCH --time=00:15:00
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=12

module purge
module load gcc/12.2.0-gcc-8.5.0-p4pe45v

gcc -std=gnu11 -Wall -Wextra -O2 -fopenmp mandelbrot_omp.c -o mandelbrot_omp

echo "===== Thread-count benchmark (static schedule) ====="
for T in 1 4 8 12; do
    ./mandelbrot_omp $T
done

echo ""
echo "===== Scheduling comparison (12 threads) ====="
# runtime schedule reads OMP_SCHEDULE; test with dynamic,1
export OMP_SCHEDULE="dynamic,1"
./mandelbrot_omp 12 schedules
