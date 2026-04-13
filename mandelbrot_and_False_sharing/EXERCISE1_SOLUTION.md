# Exercise 1 — Solution

This file is **our submitted write-up** for Exercise 1. The Mandelbrot program is **single-threaded sequential** code only: **no OpenMP**, **no threads**, **no parallelism**. We also **did not use compiler optimizations** such as **`-O2`** or **`-O3`** — only a **normal `gcc`** line (`-std=gnu11 -Wall -Wextra -lm`, same idea as the `Makefile`). On **LCC3** we ran the benchmark with **SLURM** using **`sbatch`** on a **compute node** (not the login node), with **`--cpus-per-task=1`**, so the timing matches this **plain sequential** build.

---

## Task 1 — The function we wrote

**In plain words:**  
For every pixel in the image we pick a point `(cx, cy)` on the Mandelbrot map. We run the loop from the assignment until the point “escapes” or we hit `MAX_ITER`. Then we turn the step count into a gray value (0–255) and save it in `image`.

**The function:**

```c
void calc_mandelbrot(uint8_t image[Y][X]) {

	for (int y_pixel = 0; y_pixel < Y; y_pixel++) {
		for (int x_pixel = 0; x_pixel < X; x_pixel++) {
			double x = 0.0;
			double y = 0.0;
			double cx = -2.5 + (x_pixel / (double)(X - 1)) * (1.0 - (-2.5));
			double cy = 1.0 - (y_pixel / (double)(Y - 1)) * (1.0 - (-1.0));
			int iteration = 0;
			while (x * x + y * y <= 2.0 * 2.0 && iteration < MAX_ITER) {
				double x_tmp = x * x - y * y + cx;
				y = 2.0 * x * y + cy;
				x = x_tmp;
				iteration++;
			}
			uint8_t norm_iteration = (uint8_t)((iteration * 255) / MAX_ITER);
			image[y_pixel][x_pixel] = norm_iteration;
		}
	}
}
```

---

## How we build and run the program

**Step 1 — Go to the folder**

```bash
cd mandelbrot
```

**Step 2 — Build**

```bash
make
```

This creates a program named `mandelbrot`.

**Step 3 — Run**

```bash
./mandelbrot
```

**Step 4 — What you get**  
The program writes a file named **`mandelbrot.png`** in the same folder.

---

## Task 2 — The image and why we say it is correct

**Picture from this solution:**

![Output of the Mandelbrot program](mandelbrot/mandelbrot.png)

**Why this means the implementation is correct**

- You should see the **well-known Mandelbrot shape**: a big **heart-like** region on the right (the cardioid) and smaller **round “bulbs”** around it.
- If the formulas or the mapping from pixels to `(cx, cy)` were wrong, you would usually get a **blank image**, **noise**.

---

## Task 3 — Benchmark on LCC3, spreadsheet, and how to improve speed

**What Task 3 asks:** Time the Mandelbrot program on a **compute node** (not the login node), write the numbers down, put them in the **comparison spreadsheet** from Matrix / OLAT, and explain what you would change to make it faster.

### Steps

**Step 1 — Copy your code to LCC3**  
Put the whole `mandelbrot` folder on the cluster (same files as locally: at least `mandelbrot.c`, `stb_image_write.h`).

**Step 2 — Log in and go to that folder on the login node**

```bash
ssh lcc3
cd assignment3/mandelbrot
```

**Step 3 — Use the batch script**

```bash
#!/bin/bash
#SBATCH --job-name=mandelbrot
#SBATCH --output=mandelbrot_%j.out
#SBATCH --time=00:05:00
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
# Add if your site requires it, e.g.:
#SBATCH --partition=YOUR_PARTITION

module purge
module load gcc/12.2.0-gcc-8.5.0-p4pe45v

# No -O2 / -O3: plain compile for this submission
gcc -std=gnu11 -Wall -Wextra mandelbrot.c -lm -o mandelbrot

./mandelbrot --benchmark
```

**Step 4 — Submit the job (runs on a compute node, not the login node)**

```bash
sbatch mandelbrot_job.sh
```

Slurm prints a job id, `Submitted batch job 344997`.

**Step 5 — Get the benchmark number**

```bash
cat mandelbrot_344997.out
```

**Our measured result:** after `cat` on the job’s `.out` file, the printed value was **`9.79`** seconds (wall time for **`calc_mandelbrot`** only, from **`./mandelbrot --benchmark`**, **1 thread**, with the **non-optimized** compile line above).

We entered **9.79** in the course **comparison spreadsheet** (Exercise 1: mandelbrot, **1 thread**), using a **dot** as the decimal point, as the sheet asks.

_If you change the compile flags later, run **`sbatch`** again and update both this report and the spreadsheet so they stay consistent._

---

## Task 4 — How can we improve the performance of the program?

1. **Use stronger compiler options.** Our submission uses a plain `gcc` line without **`-O2`** or **`-O3`**. Turning on optimization lets the compiler reorder and simplify the hot loops, so the same code usually runs **faster**. We would re-time on LCC3 and write the exact flags next to the new number.

2. **Use more than one CPU core.** Each pixel can be computed on its own. In a **future version**, we could add **OpenMP** (for example a parallel loop over **rows**) and compile with **`-fopenmp`**, so several cores work at the same time.

---

## Task 5 — Can you parallelize this algorithm?

**Yes.** Each pixel is **independent**, so we could use **OpenMP** on the **outer row loop** (`y_pixel`): each thread handles **different rows** of `image`, compiles with **`-fopenmp`**, and runs **`OMP_NUM_THREADS=8`**. the block below is only an **example** of how a parallel version could look.

**Code snapshot**

```c
#include <omp.h>

void calc_mandelbrot(uint8_t image[Y][X]) {
	#pragma omp parallel for schedule(static)
	for (int y_pixel = 0; y_pixel < Y; y_pixel++) {
		for (int x_pixel = 0; x_pixel < X; x_pixel++) {
			double x = 0.0;
			double y = 0.0;
			double cx = -2.5 + (x_pixel / (double)(X - 1)) * (1.0 - (-2.5));
			double cy = 1.0 - (y_pixel / (double)(Y - 1)) * (1.0 - (-1.0));
			int iteration = 0;
			while (x * x + y * y <= 2.0 * 2.0 && iteration < MAX_ITER) {
				double x_tmp = x * x - y * y + cx;
				y = 2.0 * x * y + cy;
				x = x_tmp;
				iteration++;
			}
			uint8_t norm_iteration = (uint8_t)((iteration * 255) / MAX_ITER);
			image[y_pixel][x_pixel] = norm_iteration;
		}
	}
}
```
