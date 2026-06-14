# Exercise 2 — OpenMP Parallelization

## Compile commands

**Sequential:**
```bash
gcc -O3 -std=c99 *.c -o real_seq -lm
```

**Parallel:**
```bash
gcc -O3 -fopenmp -std=c99 *.c -o real_omp -lm
```

**Run with N threads:**
```bash
OMP_NUM_THREADS=N ./real_omp
```

---

## What we did

We added `#include <omp.h>` at the top of the file, then parallelized the outer loop of the 5 hottest functions identified in the profile.

The general pattern was the same in almost every case:
- Add `#pragma omp parallel for` on the outer `i3` loop
- Move any temporary arrays (like `u1`, `u2`, `r1`, `r2`...) **from outside the loop to inside** , this is necessary because if they stay outside, all threads share them and overwrite each other (data race)

---

## Changes per function

### `resid` — 42% of runtime
```c
// before
double u1[M], u2[M];
for (i3 = 1; i3 < n3-1; i3++) {

// after
#pragma omp parallel for private(i2, i1) schedule(static)
for (i3 = 1; i3 < n3-1; i3++) {
    double u1[M], u2[M];  // each thread has its own copy
```

### `psinv` — 21% of runtime
```c
// before
double r1[M], r2[M];
for (i3 = 1; i3 < n3-1; i3++) {

// after
#pragma omp parallel for private(i2, i1) schedule(static)
for (i3 = 1; i3 < n3-1; i3++) {
    double r1[M], r2[M];
```

### `rprj3` — 10% of runtime
```c
// before
double x1[M], y1[M], x2, y2;
for (j3 = 1; j3 < m3j-1; j3++) {

// after
#pragma omp parallel for private(j2, j1, i3, i2, i1) schedule(static)
for (j3 = 1; j3 < m3j-1; j3++) {
    double x1[M], y1[M], x2, y2;
```
Note: `i3` is also listed as `private` because it is computed inside the loop from `j3`.

### `interp` — 8% of runtime
```c
// before
double z1[M], z2[M], z3[M];
if (n1 != 3 && n2 != 3 && n3 != 3) {
    for (i3 = 0; i3 < mm3-1; i3++) {

// after
if (n1 != 3 && n2 != 3 && n3 != 3) {
    #pragma omp parallel for private(i2, i1) schedule(static)
    for (i3 = 0; i3 < mm3-1; i3++) {
        double z1[M], z2[M], z3[M];
```
Only the main branch was parallelized. The `else` branch handles very small coarse grids and has negligible runtime.

### `norm2u3` — 4% of runtime
```c
// before
for (i3 = 1; i3 < n3-1; i3++) {

// after
#pragma omp parallel for reduction(+:s) private(i2, i1, a) schedule(static)
for (i3 = 1; i3 < n3-1; i3++) {
```
This is a sum reduction — the `reduction(+:s)` clause tells OpenMP to give each thread its own copy of `s` and combine them at the end.

### `zero3` — minor
```c
// before
for (i3 = 0; i3 < n3; i3++) {

// after
#pragma omp parallel for private(i2, i1) schedule(static)
for (i3 = 0; i3 < n3; i3++) {
```

---

## What was NOT parallelized

**`comm3`** copies boundary values by reading and writing the **same array** in the same pass. This creates dependencies between iterations that make it unsafe to parallelize. It also accounts for less than 1% of runtime so it is not worth the effort.
.

---

## Benchmark Results

Measurements performed on LCC3

| Version    | Threads | Time (s) | Speedup |
|------------|--------:|---------:|--------:|
| Sequential | —       | 6.86     | 1.00×   |
| OpenMP     | 1       | 6.89     | 1.00×   |
| OpenMP     | 2       | 4.84     | 1.42×   |
| OpenMP     | 6       | 2.82     | 2.43×   |
| OpenMP     | 12      | 3.02     | 2.27×   |

## Discussion

The results show limited speedup compared to what could be expected. A few observations:

- 1 thread is basically the same as sequential : expected, just adds a tiny OpenMP overhead
- 2 threads gives 1.42× speedup : parallelization is working
- 6 threads is the sweet spot at 2.43× : best result
- 12 threads is slightly slower than 6 : too many threads competing for memory access, the overhead outweighs the benefit

These loops mostly just read and write large arrays in memory. Adding more threads does not help much because they all end up waiting for data from RAM — the memory becomes the bottleneck, not the CPU.