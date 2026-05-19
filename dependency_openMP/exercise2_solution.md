# Parallelism & Loop Analysis

---

## 1) `copy(double* x, double* y)`

```c
void copy(double* x, double* y) {
    for(int i = 0; i < 1024; i++) {
        x[i] = y[i];
    }
}
```

### Can it be safely parallelized manually?

It depends on aliasing.  
If `x` and `y` point to different, non-overlapping memory areas, then yes:

```c
#pragma omp parallel for
for (int i = 0; i < 1024; i++) {
    x[i] = y[i];
}
```

Each iteration writes only to `x[i]`, so there is no conflict between iterations.

But if `x` and `y` overlap, for example:

```c
copy(a + 1, a);
```

then iteration order may matter, because one iteration may overwrite a value that another iteration still needs to read. So in the general case, manual parallelization is not always safe unless you know there is no overlap.

### Can the compiler safely parallelize it?

Usually not automatically, because the compiler cannot always prove that `x` and `y` do not alias.  
To help the compiler, you can use `restrict`:

```c
void copy(double* restrict x, double* restrict y) {
    for(int i = 0; i < 1024; i++) {
        x[i] = y[i];
    }
}
```

This tells the compiler that `x` and `y` do not overlap, enabling auto-vectorization and auto-parallelization.

---

## 2) Normalize the Loop Nest

**Original code:**

```c
for (int i = 4; i <= N; i += 9) {
    for (int j = 0; j <= N; j += 5) {
        A[i] = 0;
    }
}
```

Loop normalization means changing the loop so that the induction variables start from `0` and increase by `1`.

Let:

```
i = 4 + 9 * ii
j = 0 + 5 * jj
```

So the normalized version is:

```c
for (int ii = 0; ii <= (N - 4) / 9; ii++) {
    int i = 4 + 9 * ii;

    for (int jj = 0; jj <= N / 5; jj++) {
        int j = 5 * jj;

        A[i] = 0;
    }
}
```

Notice that `j` is not actually used inside the loop body. The inner loop therefore repeatedly writes the same value `A[i] = 0`.

A simplified equivalent version is:

```c
for (int ii = 0; ii <= (N - 4) / 9; ii++) {
    int i = 4 + 9 * ii;
    A[i] = 0;
}
```

Because writing `A[i] = 0` many times has the same final effect as writing it once.

---

## 3) Dependency Analysis

**Code:**

```c
for(int i = 1; i < N; i++) {
    for(int j = 1; j < M; j++) {
        for(int k = 1; k < L; k++) {
            a[i+1][j][k-1] = a[i][j][k] + 5;
        }
    }
}
```

We compare the write and read accesses:

```
write: a[i+1][j][k-1]
read:  a[i][j][k]
```

To find a dependency, we ask: when does a value written by one iteration get read by another?

Let the **writing** iteration be `(i1, j1, k1)`. It writes to:

```
a[i1 + 1][j1][k1 - 1]
```

Let the **reading** iteration be `(i2, j2, k2)`. It reads from:

```
a[i2][j2][k2]
```

Setting them equal:

```
i1 + 1 = i2
j1     = j2
k1 - 1 = k2
```

So:

```
i2 = i1 + 1
j2 = j1
k2 = k1 - 1
```

The reading iteration is therefore at `(i1 + 1, j1, k1 - 1)`.

### Distance and Direction Vectors

The **distance vector** from the writing iteration to the reading iteration is:

```
(i2 - i1, j2 - j1, k2 - k1) = (1, 0, -1)
```

The **direction vector** is derived from the sign of each distance component:

| Dimension | Distance | Direction |
|-----------|----------|-----------|
| `i`       | +1       | `<`       |
| `j`       | 0        | `=`       |
| `k`       | −1       | `>`       |

```
distance vector:  (1, 0, -1)
direction vector: (<, =, >)
```

### Does it carry a dependency?

Yes, this is a **loop-carried dependency** carried by the `i` and `k` loops. One iteration writes a value that a later iteration reads.

### Can we parallelize it?

Not the full loop nest directly. However, the `j` dimension has distance `0`, meaning different `j` iterations are completely independent of each other. The safest parallelization is therefore over `j`:

```c
#pragma omp parallel for
for(int j = 1; j < M; j++) {
    for(int i = 1; i < N; i++) {
        for(int k = 1; k < L; k++) {
            a[i+1][j][k-1] = a[i][j][k] + 5;
        }
    }
}
```

The inner `(i, k)` loops execute sequentially in their original order, correctly respecting the dependency `(1, 0, -1)`.
