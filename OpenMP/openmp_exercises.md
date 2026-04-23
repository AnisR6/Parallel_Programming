# Exercise 1

## 1) High-level explanation

The program shows a simple communication between two OpenMP threads using the shared variables `data` and `flag`.

- **Thread 0** writes `data = 42`, then sets `flag = 1`
- **Thread 1** keeps checking `flag` until it becomes `1`, then prints `flag` and `data`

So the intended output is:

```
flag=1 data=42
```

This is a basic producer-consumer pattern: one thread produces the data and signals that it is ready, while the other waits for that signal before reading it.

---


## 2) What happens with `-O3` and repeated runs

When compiled with `-O3` and executed many times, the program does not always behave correctly. Sometimes it prints the expected result, but sometimes it hangs forever in the loop:

```c
while (flag_val < 1)
```

The reason is that there is no proper synchronization between the two threads. Because of compiler optimization and caching, Thread 1 may keep seeing an old value of `flag` and never notice that Thread 0 updated it. This creates non-deterministic behavior.

---

## 3) Does it require `#pragma omp flush`?

**Yes.** `flush` is needed to guarantee visibility of updates between the threads.

A correct version is:

```c
#include <omp.h>
#include <stdio.h>

int main() {
    int data = 0;
    int flag = 0;

    #pragma omp parallel num_threads(2) shared(data, flag)
    {
        if (omp_get_thread_num() == 0) {
            data = 42;
            #pragma omp flush(data)

            flag = 1;
            #pragma omp flush(flag)

        } else if (omp_get_thread_num() == 1) {
            int flag_val = 0;

            while (flag_val < 1) {
                #pragma omp flush(flag)
                flag_val = flag;
            }

            #pragma omp flush(data)
            printf("flag=%d data=%d\n", flag, data);
        }
    }

    return 0;
}
```

Why these flushes are needed:

- After writing `data`, so the value becomes visible
- After writing `flag`, so the signal becomes visible
- Inside the waiting loop, so Thread 1 reloads the newest `flag`
- Before reading `data`, so Thread 1 sees the updated value



# Exercise 2

## 1)

```c
a[0] = 0;
#pragma omp parallel for
for (i = 1; i < N; i++) {
    a[i] = 2.0 * i * (i - 1);
    b[i] = a[i] - a[i - 1];
}
```

### Problem

This is **not correctly parallelized**.

`b[i]` depends on `a[i-1]`, which may belong to another iteration and may not have been computed yet. So there is a loop-carried dependence.

### Fix 1: two loops

```c
a[0] = 0;

#pragma omp parallel
{
#pragma omp for
    for (i = 1; i < N; i++) {
        a[i] = 2.0 * i * (i - 1);
    }

#pragma omp for
    for (i = 1; i < N; i++) {
        b[i] = a[i] - a[i - 1];
    }
}
```

This works because a worksharing loop has an implicit barrier at the end unless `nowait` is used.

### Fix 2: remove dependence algebraically

Since $a[i] = 2i(i-1)$, then $b[i] = a[i] - a[i-1] = 4i - 4$.

So you could do:

```c
a[0] = 0;
#pragma omp parallel for
for (i = 1; i < N; i++) {
    a[i] = 2.0 * i * (i - 1);
    b[i] = 4.0 * i - 4.0;
}
```

### Pros / cons

- **Two loops:** general and safe.
- **Algebraic rewrite:** fasy but only possible because this formula is simple, and on this specific code.

---

## 2)

```c
a[0] = 0;
#pragma omp parallel
{
#pragma omp for nowait
    for (i = 1; i < N; i++) {
        a[i] = 3.0 * i * (i + 1);
    }
#pragma omp for
    for (i = 1; i < N; i++) {
        b[i] = a[i] - a[i - 1];
    }
}
```

### Problem

This is **wrong** because of `nowait`.

`nowait` removes the barrier at the end of the first loop, so the second loop may start before all `a[i]` values are ready.
### Fix 1: remove `nowait`

```c
a[0] = 0;
#pragma omp parallel
{
#pragma omp for
    for (i = 1; i < N; i++) {
        a[i] = 3.0 * i * (i + 1);
    }

#pragma omp for
    for (i = 1; i < N; i++) {
        b[i] = a[i] - a[i - 1];
    }
}
```

### Fix 2: keep `nowait`, add barrier

```c
a[0] = 0;
#pragma omp parallel
{
#pragma omp for nowait
    for (i = 1; i < N; i++) {
        a[i] = 3.0 * i * (i + 1);
    }

#pragma omp barrier

#pragma omp for
    for (i = 1; i < N; i++) {
        b[i] = a[i] - a[i - 1];
    }
}
```

### Fix 3: algebraic rewrite

Since $a[i] = 3i(i+1)$, then $b[i] = a[i] - a[i-1] = 6i$.

```c
a[0] = 0;
#pragma omp parallel for
for (i = 1; i < N; i++) {
    a[i] = 3.0 * i * (i + 1);
    b[i] = 6.0 * i;
}
```

### Pros / cons

- **Remove `nowait` / add barrier:** general solution.
- **Formula rewrite:** simpler and faster if valid.

---

## 3)

```c
#pragma omp parallel for default(none)
for (i = 0; i < N; i++) {
    x = sqrt(b[i]) - 1;
    a[i] = x * x + 2 * x + 1;
}
```

### Problem

This is **not complete** with `default(none)`.

With `default(none)`, variables must be scoped explicitly unless they are predetermined ( when you say default(none), it means that you do not trust auto parallelization). `i` is loop-private, but `N`, `a`, `b`, and likely `x` need attention. This often causes a compiler error.

### Correct version

```c
#pragma omp parallel for default(none) shared(a, b, N) private(x)
for (i = 0; i < N; i++) {
    x = sqrt(b[i]) - 1;
    a[i] = x * x + 2 * x + 1;
}
```


each iteration writes only `a[i]` and reads only `b[i]`, so iterations are independent.


---

## 4)

```c
f = 2;
#pragma omp parallel for private(f, x)
for (i = 0; i < N; i++) {
    x = f * b[i];
    a[i] = x - 7;
}
```

### Problem

This is **wrong**.

`private(f)` gives each thread its own **uninitialized** copy of `f`, so inside each loop, `f` is undefined. this will create garbage variable ( random values from memory) and will result in bugs and wrong values.

### Fix 1: `firstprivate`

```c
f = 2;
#pragma omp parallel for firstprivate(f) private(x)
for (i = 0; i < N; i++) {
    x = f * b[i];
    a[i] = x - 7;
}
```

### Fix 2: keep `f` shared

```c
f = 2;
#pragma omp parallel for shared(f) private(x)
for (i = 0; i < N; i++) {
    x = f * b[i];
    a[i] = x - 7;
}
```

### Better style

```c
f = 2;
#pragma omp parallel for shared(a, b, N, f)
for (i = 0; i < N; i++) {
    double x = f * b[i];
    a[i] = x - 7;
}
```

### Pros / cons

- **`firstprivate(f)`:** safest if later someone changes `f` inside the loop body.
- **`shared(f)`:** fine if `f` is read-only.

---

## 5)

```c
a[0] = x;
sum = 0;
#pragma omp parallel for
for (i = 0; i < N; i++) {
    sum = sum + b[i];
}
```

### Problem

This is **wrong** due to a race on `sum`.

Multiple threads update `sum` at the same time.

### Fix 1: `reduction`
it means each thread has its own local sum:

Thread 0 → local_sum = ...
Thread 1 → local_sum = ...

Then at the end:

sum = sum0 + sum1 + ...

```c
a[0] = x;
sum = 0;

#pragma omp parallel for reduction(+:sum)
for (i = 0; i < N; i++) {
    sum = sum + b[i];
}
```

### Fix 2: `atomic`

```c
a[0] = x;
sum = 0;

#pragma omp parallel for
for (i = 0; i < N; i++) {
#pragma omp atomic update
    sum += b[i];
}
```

### Pros / cons

- **Reduction:** best performance, standard solution.
- **Atomic:** simpler conceptually, usually slower.

---

## 6)

```c
#pragma omp parallel
#pragma omp for
for (i = 0; i < N; i++) {
#pragma omp for
    for (j = 0; j < N; j++) {
        a[i][j] = b[i][j];
    }
}
```

### Problem

This is invalid OpenMP usage.

A worksharing construct like `omp for` must not be closely nested inside another worksharing construct. So this causes a compiler issue depending on the compiler.

### Fix 1: parallelize outer loop only

```c
#pragma omp parallel for
for (i = 0; i < N; i++) {
    for (j = 0; j < N; j++) {
        a[i][j] = b[i][j];
    }
}
```

### Fix 2: `collapse` both loops

It means to treat the 2 nested loops like one big loop
So instead of parallelizing only rows, it parallelizes the whole i,j iteration space.

```c
#pragma omp parallel for collapse(2)
for (i = 0; i < N; i++) {
    for (j = 0; j < N; j++) {
        a[i][j] = b[i][j];
    }
}
```

### Pros / cons

- **Outer-loop only:** simple, good if rows are large.
- **`collapse(2)`:** better load balance for some shapes, but may reduce locality a bit depending on scheduling.
