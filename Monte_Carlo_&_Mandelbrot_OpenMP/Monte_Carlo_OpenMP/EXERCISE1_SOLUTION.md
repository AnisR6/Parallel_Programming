# Exercise 1 – Monte Carlo Pi with OpenMP

## Implementation

generating 700M random points in [0,1]×[0,1] and count how many fall inside the unit circle. Since π/4 is the ratio of the circle's area to the square's area, we get π ≈ 4 * count / n.

The serial version is straightforward. The problem when parallelizing is that every thread needs to update the same `count` variable, which causes a data race if left unprotected. We implemented three ways to fix this.

We use `rand_r(&seed)` instead of `rand()` in all parallel versions. `rand()` has an internal global lock, using it in a parallel loop would serialize all random number generation and completely defeat the point of parallelizing.

---

## Critical section

```c
#pragma omp parallel for
for (i = 0; i < n; i++) {
    unsigned int seed = (unsigned int)(omp_get_thread_num() * 1000 + i);
    double x = (double)rand_r(&seed) / RAND_MAX;
    double y = (double)rand_r(&seed) / RAND_MAX;
    if (x * x + y * y <= 1) {
#pragma omp critical
        count++;
    }
}
```

Only one thread at a time can enter the critical section. Every other thread that reaches it has to stop and wait. Correct, but every increment goes through a full mutex.

## Atomic

```c
if (x * x + y * y <= 1) {
#pragma omp atomic
    count++;
}
```

Same idea but lighter. For a simple increment the compiler generates a single locked hardware instruction instead of a full mutex. Less overhead per operation than critical.

## Reduction

```c
#pragma omp parallel for reduction(+:count)
for (i = 0; i < n; i++) {
    ...
    if (x * x + y * y <= 1)
        count++;
}
```

Each thread gets its own private copy of `count` and increments it freely. At the end OpenMP sums all copies together. No synchronization happens during the loop at all.

---

## Results (LCC3, n = 700,000,000)

| Threads | serial (s) | critical (s) | atomic (s) | reduction (s) |
| ------: | ---------: | -----------: | ---------: | ------------: |
|       1 |    17.7266 |      16.3551 |    13.8968 |       10.3608 |
|       4 |          — |     109.0926 |    22.9142 |        2.6105 |
|       8 |          — |     174.2413 |    23.9066 |        1.3481 |
|      12 |          — |     185.7499 |    23.7984 |        0.9003 |

---

## Observations

**Critical** gets dramatically worse as you add threads. At 12 threads it's 185 s , that's 11× slower than with 1 thread and about 10× slower than serial.
The reason is that roughly half of all 700M iterations land inside the circle and trigger the lock. With 12 threads all competing for the same mutex hundreds of millions of times, they spend almost all their time waiting rather than computing. More threads just means a longer queue.

**Atomic** is much better than critical and does get slower with more threads, but it levels off around 23 s between 4 and 12 threads. 
The hardware atomic instruction is fast enough that the bottleneck becomes the memory bus, all threads still have to serialize through the same cache line, but the per-operation cost is low enough that adding threads beyond 4 doesn't make it much worse (or better).

**Reduction** is the only one that actually benefits from more threads. 12 threads gives 0.9 s, almost 20× faster than serial and about 200× faster than critical at the same thread count. 
The speedup is close to linear because threads are completely independent during the loop, the only shared work is the final summation which takes negligible time.

we were asked to increment `count` directly without accumulating into a private variable first. This is what makes the differences so extreme. If each thread kept a local counter and only touched `count` once at the end, even critical and atomic would scale fine. Hitting the shared variable on every single point that lands inside the circle (~350M times) is what causes the collapse.

---

## Fastest 12-thread time for the spreadsheet

**Reduction: 0.9003 s**
