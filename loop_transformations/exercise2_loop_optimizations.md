# Exercise 2 – Loop Optimizations

**Goal:** reduce loop overhead, redundant computation, and cache misses.

---

## 1 – Loop Unrolling

Do multiple iterations per loop cycle to reduce branch/compare overhead.

```c
// original
for (int i = 0; i < N - 1; ++i)
    a[i] = b[i] + b[i + 1];

// unrolled ×2  (N is odd → N-1 is even → no remainder)
for (int i = 0; i < N - 2; i += 2) {
    a[i]     = b[i]     + b[i + 1];
    a[i + 1] = b[i + 1] + b[i + 2];
}
```

Halves branch count. The two body lines are independent → CPU can run them in parallel.

---

## 2 – Loop-Invariant Code Motion (LICM)

Move a computation that never changes outside the loop.

```c
// original — hypot() called N times with the same inputs
for (int i = 0; i < N; ++i)
    a[i] *= hypot(0.3, 0.4);

// optimised — called once
double h = hypot(0.3, 0.4);
for (int i = 0; i < N; ++i)
    a[i] *= h;
```

`hypot` involves a sqrt (~20 cycles). Calling it N times is pure waste.

---

## 3 – Loop Unswitching

Move an invariant branch outside the loop.

```c
// original — N % 2 evaluated N times, result never changes
for (int i = 0; i < N; ++i) {
    if (N % 2) a[i] = b[i] + 5;
    else       a[i] = c[i] + 5;
}

// optimised — decided once
if (N % 2) {
    for (int i = 0; i < N; ++i) a[i] = b[i] + 5;
} else {
    for (int i = 0; i < N; ++i) a[i] = c[i] + 5;
}
```

Eliminates N branches. Each resulting loop is branch-free and easier to vectorise.

---

## 4 – Loop Fission / Distribution

Split one loop into several simpler ones.

```c
// original
for (int i = 0; i < N; ++i) {
    sum_a += a[i];
    sum_b += b[i];
    sum_c += c[i];
}

// fissioned
for (int i = 0; i < N; ++i) sum_a += a[i];
for (int i = 0; i < N; ++i) sum_b += b[i];
for (int i = 0; i < N; ++i) sum_c += c[i];
```

**Cache impact:** same total data touched → similar cache misses. Useful mainly when dependencies in the original prevent vectorisation. For simple reductions like this, the original fused loop is often faster.

---

## 5 – Loop Peeling + Loop Fusion

Peel one iteration to align ranges, then merge two loops into one pass.

```c
// original — two passes over a[]
int min = a[0];
for (int i = 1; i < N; ++i)
    min = (a[i] < min) ? a[i] : min;
for (int i = 0; i < N; ++i)
    sum += a[i];

// peeled + fused — one pass over a[]
int min = a[0];
sum += a[0];                              // peeled
for (int i = 1; i < N; ++i) {
    if (a[i] < min) min = a[i];
    sum += a[i];
}
```

Halves the number of memory loads on `a[]`. Halves loop overhead.

---

## 6 – Loop Splitting

Split a loop with an index-dependent branch into two stride-2 loops.

```c
// original — branch every iteration
for (int i = 0; i < N; ++i) {
    if (i % 2) a[i] = b[i] + 4;
    else       a[i] = c[i] + 5;
}

// split by parity
for (int i = 0; i < N; i += 2) a[i]     = c[i] + 5;   // even
for (int i = 1; i < N; i += 2) a[i]     = b[i] + 4;   // odd
```

Eliminates all branches inside the loop. Each loop is branch-free and vectorisable.  
Stride-2 access is slightly less cache-friendly than stride-1.

---

## 7 – Loop Tiling (Cache Blocking)

Process the matrix in small blocks that fit in L1 cache to avoid cache thrashing.

```c
// original — b[k][j] accessed column-wise → cache miss every step
for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j)
        for (int k = 0; k < N; ++k)
            c[i][j] += a[i][k] * b[k][j];

// tiled — each BLOCK_SIZE×BLOCK_SIZE tile fits in L1
for (int ii = 0; ii < N; ii += BLOCK_SIZE)
  for (int jj = 0; jj < N; jj += BLOCK_SIZE)
    for (int kk = 0; kk < N; kk += BLOCK_SIZE)
      for (int i = ii; i < ii + BLOCK_SIZE && i < N; ++i)
        for (int j = jj; j < jj + BLOCK_SIZE && j < N; ++j)
          for (int k = kk; k < kk + BLOCK_SIZE && k < N; ++k)
            c[i][j] += a[i][k] * b[k][j];
```

Reduces cache misses by ~BLOCK_SIZE factor. The single biggest win for large N.  
`BLOCK_SIZE = 64 / sizeof(double) = 8` → one tile row = one cache line.

---

## Summary

| # | Transformation | Main benefit |
|---|---|---|
| 1 | Unrolling | Fewer branches, more ILP |
| 2 | LICM | Remove redundant computation |
| 3 | Unswitching | Remove invariant branch |
| 4 | Fission | Reduce register pressure |
| 5 | Peeling + Fusion | Half the memory loads |
| 6 | Splitting | Remove index-dependent branch |
| 7 | Tiling | Drastically fewer cache misses |
