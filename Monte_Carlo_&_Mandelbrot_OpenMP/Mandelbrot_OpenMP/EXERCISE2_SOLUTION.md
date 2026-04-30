# Exercise 2 – Mandelbrot with OpenMP

## Implementation

The implementation is straightforward compared to the Pthreads version from assignment 4. Instead of manually splitting rows between threads and managing `pthread_create`/`pthread_join`, we just add one pragma above the outer loop:

```c
#pragma omp parallel for schedule(static)
for (int y_pixel = 0; y_pixel < Y; y_pixel++) {
    for (int x_pixel = 0; x_pixel < X; x_pixel++) {
        double x = 0.0, y = 0.0;
        double cx = -2.5 + (x_pixel / (double)(X - 1)) * 3.5;
        double cy =  1.0 - (y_pixel / (double)(Y - 1)) * 2.0;
        int iter = 0;
        while (x*x + y*y <= 4.0 && iter < MAX_ITER) {
            double xt = x*x - y*y + cx;
            y = 2.0*x*y + cy;
            x = xt;
            iter++;
        }
        image[y_pixel][x_pixel] = (uint8_t)((iter * 255) / MAX_ITER);
    }
}
```

We only parallelize the outer loop (over rows). Each row is completely independent, no thread writes to the same pixel as another, so there are no race conditions. The local variables `x`, `y`, `cx`, `cy`, `iter` are declared inside the loop body so they are automatically private to each thread.

Compile with:
```bash
gcc -std=gnu11 -O2 -fopenmp mandelbrot_omp.c -o mandelbrot_omp
```

---

## Thread-count benchmark (static schedule)

Measured on LCC3 with `--cpus-per-task=12`:

| Threads | Time (s) | Speedup |
| ------: | -------: | ------: |
|       1 |   9.7789 |   1.00× |
|       4 |   4.1343 |   2.36× |
|       8 |   2.5230 |   3.88× |
|      12 |   1.7618 |   5.55× |

The speedup is real but nowhere near ideal (12× for 12 threads). With `static` scheduling, OpenMP splits the 720 rows into 12 consecutive blocks of 60 rows each. The problem is that rows in the middle of the image (around rows 300–420) are inside or near the Mandelbrot set and hit the full `MAX_ITER = 10000` iteration limit, while rows at the top and bottom of the image escape after just a few iterations. So the thread that gets assigned the center block does 10–20× more work than the others and everything stalls waiting for it at the end.

This becomes obvious once you look at the scheduling results below, switching to `dynamic` cuts the 12-thread time almost in half, without changing anything in the compute logic.

---

## Scheduling methods

**static**: Before the loop starts, OpenMP divides the iterations into equal chunks and assigns them to threads. No synchronization needed during execution, so overhead is minimal. Works great when every iteration takes roughly the same time. For Mandelbrot it's the worst option because of the load imbalance explained above.

**dynamic**: Threads pick up one chunk at a time from a shared queue as soon as they finish their previous chunk. This means faster threads do more work automatically. There's a small overhead for each queue access (a lock), but it's negligible here. This is the best fit for Mandelbrot.

**guided**: Similar to dynamic, but starts with large chunks that get smaller over time. The idea is fewer queue accesses at the start (less overhead), with fine-grained balancing towards the end. For 720 rows it ends up between static and dynamic, the large initial chunks still land expensive rows on a single thread before the balancing kicks in.

**auto**: The compiler decides the schedule. On LCC3 with GCC it resolves to something equivalent to static, so the time is basically the same.

**runtime**: No schedule is baked into the binary. Instead it reads the `OMP_SCHEDULE` environment variable at startup. Useful when you want to test different schedules without recompiling. We set `OMP_SCHEDULE=dynamic,1` and got the same result as explicit dynamic.

### Results on 12 threads

| Schedule    | Time (s) |
| ----------- | -------: |
| `static`    |   1.7611 |
| `dynamic,1` |   0.8496 |
| `guided`    |   1.0267 |
| `auto`      |   1.7605 |
| `runtime`   |   0.8495 |

`dynamic,1` is the clear winner at 0.85 s, more than twice as fast as `static`. `guided` lands in the middle at 1.03 s,  it does better than static but the large early chunks still cause some imbalance. `auto` is essentially identical to `static` on this compiler/platform. `runtime` with `OMP_SCHEDULE=dynamic,1` gives the exact same result as explicit dynamic, as expected.

The takeaway is that for workloads where iteration cost varies a lot (like Mandelbrot), `dynamic` is the right choice. For uniform workloads `static` is fine and has lower overhead.

---

## Comparison with previous implementations

Fastest 12-thread time: **0.8496 s** (`dynamic,1`)

| Implementation          | Threads | Time (s) |
| ----------------------- | ------: | -------: |
| Serial (assignment 3)   |       1 |   9.7789 |
| Pthreads (assignment 4) |      12 |      — |
| OpenMP static           |      12 |   1.7618 |
| OpenMP dynamic,1        |      12 |   0.8496 |

The Pthreads version from assignment 4 used a static row block split, so it should be in a similar range to the OpenMP static result. The advantage of OpenMP is not just performance, the code is much simpler and the scheduling strategy can be changed with a single keyword.
