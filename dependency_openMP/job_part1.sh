#!/bin/bash
#SBATCH --job-name=part1
#SBATCH --output=part1_output_%j.txt
#SBATCH --error=part1_error_%j.txt
#SBATCH --time=00:05:00
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=6
#SBATCH --mem=1G

module load gcc/12.2.0-gcc-8.5.0-p4pe45v

export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK

echo "Compiling exo2_1.c..."
gcc -O2 -fopenmp exo2_1.c -o exo2_1

if [ $? -ne 0 ]; then
    echo "Compilation failed."
    exit 1
fi

echo "Running with OMP_NUM_THREADS=$OMP_NUM_THREADS"
./exo2_1