# Exercise 1 – Data Dependency Analysis & Parallelization

---

## Snippet A

```c
for (int i = 0; i < n-1; i++) {
    x[i] = (y[i] + x[i+1]) / 7;
}
```

S1 writes `x[i]` and reads `x[i+1]`. Iteration `i` reads `x[i+1]`, which iteration `i+1` will later overwrite. So iteration `i+1` must not write `x[i+1]` before iteration `i` has read it.

**S1 δ⁻¹ S1** on array `x` — loop-carried anti-dependence, distance vector **(1)**, direction vector **(<)**.

Anti-dependences can always be eliminated by variable renaming. We take a snapshot of `x` before the loop so all iterations read from the original values:

```c
double *x_old = malloc(n * sizeof(double));
memcpy(x_old, x, n * sizeof(double));

#pragma omp parallel for
for (int i = 0; i < n-1; i++) {
    x[i] = (y[i] + x_old[i+1]) / 7;
}
free(x_old);
```

---

## Snippet B

```c
for (int i = 0; i < n; i++) {
    a = (x[i] + y[i]) / (i+1);
    z[i] = a;
}
```

S1 writes scalar `a`, S2 reads `a` within the same iteration — **loop-independent true dependence S1 δ S2**, distance **(0)**, direction **(=)**. Because `a` is shared across iterations there is also a **loop-carried output dependence S1 δ⁰ S1** and a **loop-carried anti-dependence S2 δ⁻¹ S1**, both with distance **(1)** and direction **(<)**.

The intra-iteration ordering (S1 before S2) must be preserved. The loop-carried dependences on `a` prevent direct parallelization but are eliminated by making `a` private — each thread gets its own copy:

```c
#pragma omp parallel for private(a)
for (int i = 0; i < n; i++) {
    a = (x[i] + y[i]) / (i + 1);
    z[i] = a;
}
```

---

## Snippet C

```c
f = sqrt(a + k);
for (int i = 0; i < n; i++) {
    x[i] = y[i] * 2 + b * i;
}
```

`f = sqrt(a + k)` executes before the loop and is not referenced inside it — no dependence. S1 writes `x[i]` and reads `y[i]`, `b`, and `i`. No two iterations touch the same memory location.

**No data dependences of any kind.** The loop is fully parallel:

```c
f = sqrt(a + k);

#pragma omp parallel for
for (int i = 0; i < n; i++) {
    x[i] = y[i] * 2 + b * i;
}
```

---

## Snippet D

```c
for (int i = 0; i < n; i++) {
    y[i] = x[i] + a / (i+1);
}
```

S1 writes `y[i]` and reads `x[i]` and scalar `a`. `a` is read-only. No two iterations access the same memory location.

**No data dependences of any kind.** The loop is fully parallel:

```c
#pragma omp parallel for
for (int i = 0; i < n; i++) {
    y[i] = x[i] + a / (i + 1);
}
```
