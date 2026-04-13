#!/bin/bash
#SBATCH --job-name=false-sharing
#SBATCH --output=false-sharing_%j.out
#SBATCH --error=false-sharing_%j.err
#SBATCH --time=00:10:00
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --exclusive

# when we used cpu per task is 6, the code showed us false arguments , 

module purge
module load gcc/12.2.0-gcc-8.5.0-p4pe45v

gcc -O3 -fopenmp false_sharing.c -o false_sharing
gcc -O3 -fopenmp false_sharing_2.c -o false_sharing_2

echo "===== 1 CPU / same socket ====="
OMP_NUM_THREADS=6 GOMP_CPU_AFFINITY=0,1,2,3,4,5 ./false_sharing 100000000
OMP_NUM_THREADS=6 GOMP_CPU_AFFINITY=0,1,2,3,4,5 ./false_sharing_2 100000000

echo "===== perf LLC / 1 CPU ====="
OMP_NUM_THREADS=6 GOMP_CPU_AFFINITY=0,1,2,3,4,5 perf stat -e LLC-load-misses -e LLC-store-misses ./false_sharing 100000000
OMP_NUM_THREADS=6 GOMP_CPU_AFFINITY=0,1,2,3,4,5 perf stat -e LLC-load-misses -e LLC-store-misses ./false_sharing_2 100000000

echo "===== perf L1 / 1 CPU ====="
OMP_NUM_THREADS=6 GOMP_CPU_AFFINITY=0,1,2,3,4,5 perf stat -e L1-dcache-load-misses -e L1-dcache-store-misses ./false_sharing 100000000
OMP_NUM_THREADS=6 GOMP_CPU_AFFINITY=0,1,2,3,4,5 perf stat -e L1-dcache-load-misses -e L1-dcache-store-misses ./false_sharing_2 100000000

echo "===== 2 CPUs / two sockets ====="
OMP_NUM_THREADS=6 GOMP_CPU_AFFINITY=0,1,2,12,13,14 ./false_sharing 100000000
OMP_NUM_THREADS=6 GOMP_CPU_AFFINITY=0,1,2,12,13,14 ./false_sharing_2 100000000

echo "===== perf LLC / 2 CPUs ====="
OMP_NUM_THREADS=6 GOMP_CPU_AFFINITY=0,1,2,12,13,14 perf stat -e LLC-load-misses -e LLC-store-misses ./false_sharing 100000000
OMP_NUM_THREADS=6 GOMP_CPU_AFFINITY=0,1,2,12,13,14 perf stat -e LLC-load-misses -e LLC-store-misses ./false_sharing_2 100000000

echo "===== perf L1 / 2 CPUs ====="
OMP_NUM_THREADS=6 GOMP_CPU_AFFINITY=0,1,2,12,13,14 perf stat -e L1-dcache-load-misses -e L1-dcache-store-misses ./false_sharing 100000000
OMP_NUM_THREADS=6 GOMP_CPU_AFFINITY=0,1,2,12,13,14 perf stat -e L1-dcache-load-misses -e L1-dcache-store-misses ./false_sharing_2 100000000