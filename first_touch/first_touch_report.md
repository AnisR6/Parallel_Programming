# Exercise 09 : First Touch with OpenMP




## 1. Explaning program

The program creates a big table (matrix) of numbers and runs four steps on it
| Step | What happens |
|---|---|
| **Allocate** | Reserve memory for the table |
| **Initialise** | Fill every cell with a value (`row + column`) |
| **Compute** | Add up all the values in the table |
| **Free** | Release the memory when done |

Each step is timed so we can see exactly how long it takes.

### Original code (no parallelism yet)

```c
// Fill the table, one cell at a time, one thread only
for (int i = 0; i < N; i++)
    for (int j = 0; j < N; j++)
        matrix[i][j] = i + j;

// Add up all values, one at a time, one thread only
long long sum = 0;
for (int i = 0; i < N; i++)
    for (int j = 0; j < N; j++)
        sum += matrix[i][j];
```

This is slow because only one thread does all the work. We want to use 12 threads at the same time.

---

## 2. Making it parallel :


### The three loops we change

```c
// fill the table in parallel
#pragma omp parallel for schedule(static)
for (int i = 0; i < N; i++)
    for (int j = 0; j < N; j++)
        matrix[i][j] = (long long)(i + j);

// add up values in parallel
long long sum = 0;
#pragma omp parallel for schedule(static) reduction(+:sum)
for (int i = 0; i < N; i++)
    for (int j = 0; j < N; j++)
        sum += matrix[i][j];

// free memory in parallel
#pragma omp parallel for schedule(static)
for (int i = 0; i < N; i++)
    free(matrix[i]);
```

### Why we kept (allocate) serial?

`malloc` only marks memory as reserved ( reserving pointers..) so does not actually write any data. It is very fast, so making it parallel would not help.

### Explaning `reduction(+:sum)` :

All 12 threads need to add to the same `sum` variable. Without `reduction`, they would all write to `sum` at the same time and corrupt the result (this is called a **data race**). With `reduction`, each thread keeps its own private copy of `sum`, and at the end the runtime adds all copies together safely.

### Explaning  `schedule(static)` :

`schedule(static)` gives each thread a fixed, predictable block of rows. Thread 0 always gets the first block, thread 1 always gets the second block, and so on. This predictability is crucial for first touch .

---

## 3. How do we pin each thread to one core?

### The problem

By default, the operating system can move a thread from one CPU core to another at any time. This is a problem because a thread might fill memory on one chip, then get moved to a different chip to read that same memory , which is much slower.

### The fix


```bash
export OMP_NUM_THREADS=12      # use exactly 12 threads
export OMP_PROC_BIND=close     # pin each thread to the nearest free core
export OMP_PLACES=cores        # each thread gets its own CPU core ( becaus threads may move between cores )
```


---

### A quick explanation


```
malloc()         →  just a reservation, no real RAM yet
first write      →  reservation
later reads      →  fast if same chip, SLOW if different chip
```

This is the **first-touch rule**: whoever writes first decides where the memory lives.


### Picture of the difference

```
case 1 ( the first touch version ):  memory is spread across chips:

  Chip 0  │ rows 0 – 3333    │  ← threads 0–3 write and read here
  Chip 1  │ rows 3334 – 6666 │  ← threads 4–7 write and read here
  Chip 2  │ rows 6667 – 9999 │  ← threads 8–11 write and read here
  → all reads are local = FAST

case 2 :  all memory is on one chip:
  Chip 0  │ ALL rows          │  ← only master thread wrote here
  Chip 1  │ (nothing)         │  ← threads 4–7 must fetch from Chip 0 = SLOW
  Chip 2  │ (nothing)         │  ← threads 8–11 must fetch from Chip 0 = SLOW
```


---

## 4. Does the loop schedule matter for first touch?

Yes, it does.

First touch only helps if the same thread that writes the data is also the same thread that reads it later.

The schedule decides which thread works on which rows.

---

## `schedule(static)` :  Good for First Touch

```c
#pragma omp parallel for schedule(static)
```

With static scheduling, each thread always gets the same rows.

Example with 4 threads:

- Thread 0 → rows 0–24
- Thread 1 → rows 25–49
- Thread 2 → rows 50–74
- Thread 3 → rows 75–99

This happens in BOTH loops:

1. Initialization loop
2. Computation loop

So:

- Thread 2 writes rows 50–74
- Later Thread 2 also reads rows 50–74

The data stays close to the same thread ,  so memory access is fast.

---

## `schedule(dynamic)` : Bad for First Touch

```c
#pragma omp parallel for schedule(dynamic)
```

With dynamic scheduling, threads take rows from a shared queue.

The rows assigned to each thread can change between loops.

Example:

- During initialization:
  - Thread 2 writes row 50
- During computation:
  - Thread 1 reads row 50

Now Thread 1 accesses memory created by another thread/chip, which is slower.

So even if initialization was parallel, the first-touch benefit is lost.


---

Reference: 
some part from a video explanation : https://www.youtube.com/watch?v=7rZa3C7lnGE&t=537s
Claude helped us write some code and organise this report.
