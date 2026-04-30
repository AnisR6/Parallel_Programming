#!/bin/bash
#SBATCH --job-name=monte_carlo_pi
#SBATCH --output=monte_carlo_%j.out
#SBATCH --time=00:20:00
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=12

module purge
module load gcc/12.2.0-gcc-8.5.0-p4pe45v

make clean && make

echo "===== Serial ====="
./serial

echo ""
echo "===== Critical ====="
for T in 1 4 8 12; do
    echo -n "Threads $T: "
    ./critical $T
done

echo ""
echo "===== Atomic ====="
for T in 1 4 8 12; do
    echo -n "Threads $T: "
    ./atomic $T
done

echo ""
echo "===== Reduction ====="
for T in 1 4 8 12; do
    echo -n "Threads $T: "
    ./reduction $T
done
