# Exercise 3 – Distance & Direction Vectors

---

## Code

```c
for (int i = 0; i < 4; ++i) {
    for (int j = 1; j < 4; ++j) {
        a[i + 2][j - 1] = b * a[i][j] + 4;
    }
}
```

S1 has two references to `a`: it writes `a[i+2][j-1]` and reads `a[i][j]`.

---

## Distance and Direction Vectors for Each Iteration

A dependence between two instances S1[i', j'] (source, write) and S1[i, j] (sink, read) exists when both access the same array element:

```
i' + 2 = i   →   i' = i - 2
j' - 1 = j   →   j' = j + 1
```

So instance S1[i-2, j+1] writes the value that S1[i, j] reads. S1 is both source and sink: **S1 δ S1** (true dependence).

The distance vector is defined as d(i,j)_k = sink_k − source_k:

```
d1 = i − i' = i − (i − 2) = 2
d2 = j − j' = j − (j + 1) = −1
```

**Distance vector: (2, −1)**

The direction vector encodes the sign of each component ("<" if > 0, "=" if = 0, ">" if < 0):

**Direction vector: (<, >)**

The valid source instances must satisfy `0 ≤ i' < 4` and `1 ≤ j' < 4`, meaning `i ≥ 2` and `j ≤ 2`:

| Source (i', j') | Sink (i, j) | Distance vector | Direction vector |
|---|---|---|---|
| (0, 2) | (2, 1) | (2, −1) | (<, >) |
| (0, 3) | (2, 2) | (2, −1) | (<, >) |
| (1, 2) | (3, 1) | (2, −1) | (<, >) |
| (1, 3) | (3, 2) | (2, −1) | (<, >) |

All four dependence instances share the same distance vector **(2, −1)** and direction vector **(<, >)**.

---

## Type of Dependence

This is a **true (flow) dependence** — **S1 δ S1**. S1[i', j'] writes `a[i'+2][j'-1]` and a later instance S1[i, j] reads that same element as `a[i][j]`. The source executes before the sink in lexicographic (loop) order since `i' < i`.

The dependence is **loop-carried at level 1** (the outer `i` loop). The leftmost non-"=" component of the direction vector (<, >) is "<" at position 1, which is the definition of a level-1 loop-carried dependence.

---

## Parallelization

The outer `i` loop carries the dependence — iteration `i` depends on iteration `i' = i−2` having completed, so it must stay sequential.

The inner `j` loop does not carry any dependence. For a fixed `i`, the direction vector component for `j` is ">", meaning the source would have to come from a later `j` iteration — no such dependence exists within a single `i`. All `j` iterations for a given `i` are fully independent.

**Parallelize the inner `j` loop:**

```c
for (int i = 0; i < 4; ++i) {
    #pragma omp parallel for
    for (int j = 1; j < 4; ++j) {
        a[i + 2][j - 1] = b * a[i][j] + 4;
    }
}
```

The outer `i` loop remains sequential to respect the loop-carried true dependence with distance 2.
