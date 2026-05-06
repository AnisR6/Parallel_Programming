#!/bin/bash
#SBATCH --job-name=exercise2_deps
#SBATCH --output=exercise2_%j.out
#SBATCH --time=00:10:00
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=12

module purge
module load gcc/12.2.0-gcc-8.5.0-p4pe45v

gcc -O2 -fopenmp -lm exo2.c -o exo2

./exo2
