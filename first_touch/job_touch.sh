#!/bin/bash
#SBATCH --job-name=parallel_first_touch
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=12
#SBATCH --time=00:10:00
#SBATCH --output=parallel_first_touch_%j.out

module load gcc
gcc -O2 -fopenmp -o parallel_first_touch parallel_first_touch.c

export OMP_NUM_THREADS=12
export OMP_PROC_BIND=close
export OMP_PLACES=cores

echo "--- case 1: parallel init ---"
./parallel_first_touch 40000

echo "--- case 2: serial init ---"
FIRST_TOUCH_BAD=1 ./parallel_first_touch 40000
