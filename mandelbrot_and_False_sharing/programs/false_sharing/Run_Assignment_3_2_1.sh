#!/bin/bash
#SBATCH --job-name=perf-false-sharing
#SBATCH --output=perf-false-sharing_%j.out
#SBATCH --error=perf-false-sharing_%j.err
#SBATCH --time=00:10:00
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=6

module purge
module load gcc/12.2.0-gcc-8.5.0-p4pe45v

# compile
gcc -O3 -fopenmp false_sharing.c -o false_sharing
gcc -O3 -fopenmp false_sharing_2.c -o false_sharing_2

echo "===== perf stat : false_sharing ====="
OMP_NUM_THREADS=6 perf stat ./false_sharing 100000000

echo "===== perf stat : false_sharing_2 ====="
OMP_NUM_THREADS=6 perf stat ./false_sharing_2 100000000