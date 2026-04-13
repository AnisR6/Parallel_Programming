#!/bin/bash
#SBATCH --job-name=mandelbrot
#SBATCH --output=mandelbrot_%j.out
#SBATCH --time=00:05:00
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
# If your site requires a partition, add a line:  #SBATCH --partition=<name>

module purge
module load gcc/12.2.0-gcc-8.5.0-p4pe45v

# Plain compile (no -O2 / -O3) — matches submitted report
gcc -std=gnu11 -Wall -Wextra mandelbrot.c -lm -o mandelbrot

./mandelbrot --benchmark
