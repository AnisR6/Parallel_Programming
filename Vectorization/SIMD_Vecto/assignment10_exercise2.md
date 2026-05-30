# Assignment 10 – Exercise 2: OpenMP SIMD Vectorization

## 1. Goal

The goal of this exercise was to manually vectorize the same multiply-add kernel from Exercise 1 using OpenMP SIMD, without relying on the compiler's auto-vectorization flag.

---

## 2. Implementation

I reused the same program structure as Exercise 1. The only change is adding `#pragma omp simd` before the inner loop:

```c
for (int run = 0; run < repetitions; ++run)
{
    #pragma omp simd
    for (int i = 0; i < size; ++i)
    {
        a[i] += b[i] * c[i];
    }
}
```

I also made a second version using `double` instead of `float` to observe the effect of data type on performance.

The vectors were initialized the same way as Exercise 1:
- `a[i] = 1.0`
- `b[i] = 2.0`
- `c[i] = 3.0`

Expected result: `a[i] = 1 + 1,000,000 * 6 = 6,000,001`

---

## 3. Compilation

```bash
# Sequential baseline (same as Exercise 1)
gcc -O1 -fopenmp autovectorize.c -o vector_seq

# OpenMP SIMD float (NO -ftree-vectorize)
gcc -O1 -fopenmp ex2_float.c -o vector_omp_float

# OpenMP SIMD double
gcc -O1 -fopenmp ex2_double.c -o vector_omp_double
```

---

## 4. Benchmark Results

### float

| Size | Sequential (s) | OMP SIMD (s) | Speedup |
|---:|---:|---:|---:|
| 128  | 0.171564 | 0.032663 | 5.25x |
| 256  | 0.339048 | 0.064146 | 5.29x |
| 512  | 0.686760 | 0.133836 | 5.13x |
| 1024 | 1.347385 | 0.259656 | 5.19x |
| 2048 | 2.691216 | 0.511844 | 5.26x |

### double

| Size | Sequential (s) | OMP SIMD double (s) | Speedup |
|---:|---:|---:|---:|
| 128  | 0.171564 | 0.084696 | 2.03x |
| 256  | 0.339048 | 0.135100 | 2.51x |
| 512  | 0.686760 | 0.261419 | 2.63x |
| 1024 | 1.347385 | 0.513619 | 2.62x |
| 2048 | 2.691216 | 1.583653 | 1.70x |

---

## 5. Observations

The OpenMP SIMD float version achieves about **5.2x speedup**, nearly identical to the auto-vectorized version from Exercise 1. The result is correct in all cases:
- `a[0] = 6000001.000000`
- `a[last] = 6000001.000000`

The double version is roughly **2x slower than the float version** on average, which is expected since a 128-bit XMM register fits 4 floats but only 2 doubles. Notably at size 2048 the speedup drops to only **1.70x**, which suggests that at larger sizes the double version also suffers more from increased memory pressure — each element is twice as large, putting more strain on the cache.

---

## 6. Perf Verification

```bash
# float — should show packed single-precision ops
perf stat -e r1010 ./vector_omp_float 2048

# double — should show packed double-precision ops
perf stat -e r8010 ./vector_omp_double 2048
```

| Version | r1010 (packed float) | r8010 (packed double) |
|---|---:|---:|
| OMP SIMD float  | 1,024,184,542 | ~0 |
| OMP SIMD double | ~0 | 2,058,594,581 |

This confirms that the correct SIMD instructions are emitted in both cases.

---

## 7. Comparison with Exercise 1

| Version | Time at size 2048 (s) | Speedup |
|---|---:|---:|
| Sequential      | 2.691216 | 1.00x |
| Auto-vec (Ex 1) | 0.517917 | 5.20x |
| OMP SIMD (Ex 2) | 0.511844 | 5.26x |

Performance is essentially the same. The difference is that `#pragma omp simd` gives explicit per-loop control and is portable across compilers, while `-ftree-vectorize` applies to the whole compilation unit silently.

