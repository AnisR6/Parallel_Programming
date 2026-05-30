# Assignment 10 – Exercise 1: Auto-Vectorization

## 1. Goal

The goal of this exercise was to compare the performance of a simple multiply-add kernel

```c
a[i] += b[i] * c[i];
```

using a sequential baseline and a compiler auto-vectorized version.

---

## 2. Implementation

I implemented a small C program that allocates three float vectors `a`, `b`, and `c`, initializes them with constant values, and then repeats the computation below one million times:

```c
for (int run = 0; run < repetitions; ++run) {
    for (int i = 0; i < size; ++i) {
        a[i] += b[i] * c[i];
    }
}
```

The vectors were initialized as:

- `a[i] = 1.0f`
- `b[i] = 2.0f`
- `c[i] = 3.0f`

This makes the result easy to verify. Since each iteration adds `2 * 3 = 6`, the final value is:

```text
a[i] = 1 + 1,000,000 * 6 = 6,000,001
```

I printed `a[0]` and `a[last]` to verify correctness.

---

## 3. Source code

```c
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

static void init_arrays(float *a, float *b, float *c, int size)
{
    for (int i = 0; i < size; ++i)
    {
        a[i] = 1.0f;
        b[i] = 2.0f;
        c[i] = 3.0f;
    }
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage: %s <size>\n", argv[0]);
        return 1;
    }

    int size = atoi(argv[1]);
    int repetitions = 1000000;

    float *a = (float *)malloc(size * sizeof(float));
    float *b = (float *)malloc(size * sizeof(float));
    float *c = (float *)malloc(size * sizeof(float));

    if (a == NULL || b == NULL || c == NULL)
    {
        printf("Memory allocation failed\n");
        return 1;
    }

    init_arrays(a, b, c, size);

    double start = omp_get_wtime();

    for (int run = 0; run < repetitions; ++run)
    {
        for (int i = 0; i < size; ++i)
        {
            a[i] += b[i] * c[i];
        }
    }

    double end = omp_get_wtime();

    printf("a[0] = %f\n", a[0]);
    printf("a[last] = %f\n", a[size - 1]);
    printf("Computation time: %f seconds\n", end - start);

    free(a);
    free(b);
    free(c);

    return 0;
}
```

---

## 4. Compilation

The baseline version was compiled with GCC 12.2.0 using `-O1`:

```bash
gcc -O1 -fopenmp vector.c -o vector
```

The auto-vectorized version was compiled with `-O1` and vectorization enabled:

```bash
gcc -O1 -fopenmp -ftree-vectorize vector.c -o vector_vec
```

I used `-fopenmp` because the program measures time with `omp_get_wtime()`.

---

## 5. Benchmark results

The measurements were taken on the LCC3 compute node.

### Sequential baseline

| Size | Time (s) |
|---|---:|
| 128  | 0.172694 |
| 256  | 0.340019 |
| 512  | 0.675371 |
| 1024 | 1.347345 |
| 2048 | 2.692293 |

### Auto-vectorized version

| Size | Time (s) |
|---|---:|
| 128  | 0.032613 |
| 256  | 0.063983 |
| 512  | 0.132903 |
| 1024 | 0.258646 |
| 2048 | 0.517917 |

### Speedup

| Size | Speedup |
|---|---:|
| 128  | 5.29x |
| 256  | 5.31x |
| 512  | 5.08x |
| 1024 | 5.21x |
| 2048 | 5.20x |

---

## 6. Observations

The auto-vectorized version is consistently faster than the sequential baseline by about 5x.

The result is still correct in both versions:
- `a[0] = 6000001.000000`
- `a[last] = 6000001.000000`

The performance gain is fairly stable across the tested problem sizes. The time increases with the vector length, as expected, because more elements must be processed in each repetition.

---

## 7. Perf verification

To verify that the compiler vectorized the loop, I used `perf` with the event:

```bash
perf stat -e r1010 ./vector_vec 2048
```

This event counts packed floating-point SIMD operations.

For the vectorized version, `perf` reported:

- `1,024,036,323 r1010:u`

For the sequential version, `perf` reported:

- `0 r1010:u`

This confirms that the auto-vectorized version uses SIMD instructions, while the sequential version does not.

---

