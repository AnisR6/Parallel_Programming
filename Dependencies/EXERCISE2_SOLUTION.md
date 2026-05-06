# Exercise 2 

Each snippet below is analyzed for (1) data dependencies, (2) parallelization strategy, and (3) measured wall-time results.

---

## a) Loop with a recurrence on `factor`

```c
double factor = 1;
for (int i = 0; i < n; i++) {
    x[i] = factor * y[i];
    factor = factor / 2;
}
```

### 1. Data Dependency

`factor` is a **loop-carried dependency**: each iteration reads the value of `factor` written by the previous one (`factor = factor / 2`). This is a **RAW (Read-After-Write)** dependency across iterations, making the loop inherently sequential in its original form.

### 2. Parallelization Strategy

The recurrence is eliminated by replacing `factor` with its **closed-form expression**:

$$\text{factor at iteration } i = \left(\frac{1}{2}\right)^i = 0.5^i$$

This removes the dependency entirely — each iteration becomes independent.

```c
// Parallelized
#pragma omp parallel for
for (int i = 0; i < n; i++) {
    x[i] = pow(0.5, i) * y[i];
}
```

### 3. Results

| Version    | Wall Time  |
|------------|-----------|
| Sequential | 0.2369 s  |
| Parallel   | 1.1309 s  |

**Analysis:** The parallel version is significantly **slower** here. The bottleneck is `pow(0.5, i)` and it is an expensive transcendental function called `N = 100,000,000` times, and its cost far outweighs any parallelization gain. The sequential version avoids this by using a single cheap division per iteration. 

> **Conclusion:** The parallelization is *correct* but not beneficial for this workload. In practice, one would precompute the `pow` values or use a different mathematical approach.

---

## b) Two-array loop with cross-array dependency

```c
for (int i = 1; i < n; i++) {
    x[i] = (x[i] + y[i-1]) / 2;
    y[i] = y[i] + z[i] * 3;
}
```

### 1. Data Dependency

There is a **loop-carried RAW dependency on `y`**: the update of `x[i]` reads `y[i-1]`, which is written at iteration `i-1` (`y[i-1] = y[i-1] + z[i-1] * 3`). If the two statements execute in a different order across iterations, the result changes.

- `x[i]` depends on: `y[i-1]` (previous iteration) → **loop-carried**
- `y[i]` depends on: `z[i]` only → **no cross-iteration dependency**

### 2. Parallelization Strategy

Split into **two separate parallel loops** with an implicit barrier between them. The first loop uses the *original* values of `y` (before any update), which is correct because all `y[i]` updates happen in the second loop after the barrier.

```c
// Loop 1: uses original y — safe to parallelize
#pragma omp parallel for
for (int i = 1; i < n; i++) {
    x[i] = (x[i] + y[i-1]) / 2;
}
// Implicit barrier (end of parallel for)

// Loop 2: fully independent — safe to parallelize
#pragma omp parallel for
for (int i = 1; i < n; i++) {
    y[i] = y[i] + z[i] * 3;
}
```

### 3. Results

| Version    | Wall Time  |
|------------|-----------|
| Sequential | 0.3699 s  |
| Parallel   | 0.4481 s  |

**Analysis:** The parallel version is slightly **slower**. This loop is heavily **memory-bandwidth bound** , it accesses four large arrays (`x`, `y`, `z`) with mostly sequential, cache-friendly access patterns. At `N = 10^8`, the working set (~2.4 GB) far exceeds any CPU cache, so threads compete for memory bandwidth rather than computing in parallel. Additionally, splitting into two loops doubles the number of full-array passes over memory.

> **Conclusion:** The parallelization is correct but memory bandwidth is the bottleneck, not CPU compute. Gains would appear for compute-heavy workloads or smaller arrays that fit in cache.

---

## c) Loop with conditional write-back

```c
x[0] = x[0] + 5 * y[0];
for (int i = 1; i < n; i++) {
    x[i] = x[i] + 5 * y[i];
    if (twice) {
        x[i-1] = 2 * x[i-1];
    }
}
```

### 1. Data Dependency (when `twice == true`)

There is a **loop-carried RAW + WAW dependency on `x`**:

- Iteration `i` writes `x[i]` (first statement), then writes `x[i-1]` (second statement, doubling it).
- Iteration `i+1` reads `x[i]` (to double it), which was already written by iteration `i`.

This means `x[i]` is written **twice**: once at iteration `i` and once at iteration `i+1`. The final value of `x[i]` is `2 * (x[i] + 5 * y[i])` for all `i` in `[0, n-2]`, and `x[n-1] + 5 * y[n-1]` for the last element (no subsequent iteration doubles it).

### 2. Parallelization Strategy

**Fuse** the two sequential writes on each element analytically, eliminating the recurrence:

| Element index | Final value |
|---|---|
| `i` ∈ [0, n−2] | `2 * (x[i] + 5 * y[i])` |
| `i = n−1` | `x[n-1] + 5 * y[n-1]` |

```c
// Parallelized (twice = true)
#pragma omp parallel for
for (int i = 0; i < n - 1; i++) {
    x[i] = 2 * (x[i] + 5 * y[i]);
}
x[n-1] = x[n-1] + 5 * y[n-1]; 
```

### 3. Results

| Version    | Wall Time  |
|------------|-----------|
| Sequential | 0.4105 s  |
| Parallel   | 0.2209 s  |

**Analysis:** The parallel version is around 1.8× faster , the best speedup among the three snippets. The parallelization is effective here because:

1. The fused formula is **purely arithmetic** (no expensive transcendental functions like `pow`).
2. Each iteration accesses only `x[i]` and `y[i]` , a **simple, cache-friendly** access pattern with no cross-thread conflicts.
3. The sequential version had non-trivial control flow (the `if (twice)` branch with a write-back), which the parallel version completely eliminates.

> **Conclusion:** This is the most successful parallelization. Analytical fusion of the loop-carried dependency removed the sequential bottleneck and exposed clean data parallelism.

