# Exercise 2 Solution

## Q1) How is the speedup of a parallel program defined?

### Answer

The speedup of a parallel program measures how much faster a program runs when executed in parallel compared to its sequential execution.

Using the concept from **Parallel Computing**:

$$
\text{Speedup} = \frac{\text{time with 1 processor}}{\text{time with } N \text{ processors}}
$$

$$
S(p) = \frac{T_1}{T_p}
$$

- $T_1$: time to finish the task with one processor
- $T_p$: time to finish the task with $p$ processors

---

## Q2) What is Amdahl's law and why is it important?

### Answer

The speedup might seem linear (more processors → proportionally faster), but in practice it is not. Amdahl's law explains why.

**Two factors affect speedup:**

- How much of the program runs **serially** (one step after another)
- How much runs **in parallel** (can be split across processors)

Examples of serial structures: arrays, linked lists (traversing step by step).

Examples of parallel structures: graphs, trees (different branches can run at once).

**Setting up the model:**

Total time on a single processor = 1:

$$
1 = T_s + T_p
$$

- $T_s$ = sequential (serial) part
- $T_p$ = parallel part

So $T_s = 1 - T_p$.

**Time on $n$ processors:**

The parallel part is divided by $n$, the sequential part stays the same:

$$
T_n = T_s + \frac{T_p}{n} = (1 - T_p) + \frac{T_p}{n}
$$

**Speedup with $n$ processors:**

$$
S(n) = \frac{T_1}{T_n} = \frac{1}{T_n} = \frac{1}{(1 - T_p) + \frac{T_p}{n}}
$$

Using $f = T_s$ (fraction that is sequential) and $1 - f = T_p$:

$$
S(n) = \frac{1}{f + \frac{1 - f}{n}}
$$

This is Amdahl's law. It shows that the sequential fraction $f$ limits how much speedup we can get, no matter how many processors we add.

---

## Q3) Compute speedup for 10% sequential part ($f = 0.10$)

### Answer

With 10% sequential, the parallel part is 90%: $1 - f = 0.9$.

**For 6 cores:**

$$
S(6) = \frac{1}{(1 - 0.9) + \frac{0.9}{6}} = \frac{1}{0.10 + 0.15} = \frac{1}{0.25} = 4
$$

**For unlimited cores:**

With infinitely many cores, speedup is limited by the sequential part:

$$
S(\infty) = \frac{1}{f} = \frac{1}{0.10} = 10
$$

**Final answer:**

- $S(6) = 4$
- $S(\infty) = 10$ (10× faster)

---

## Q4) Compute speedup for 20% sequential part ($f = 0.20$)

### Answer

With 20% sequential, the parallel part is 80%: $1 - f = 0.8$.

**For 6 cores:**

$$
S(6) = \frac{1}{(1 - 0.8) + \frac{0.8}{6}} = \frac{1}{0.20 + 0.1333} = \frac{1}{0.3333} \approx 3
$$

**For unlimited cores:**

$$
S(\infty) = \frac{1}{0.20} = 5
$$

**Final answer:**

- $S(6) = 3$
- $S(\infty) = 5$ (about 5× faster)

---

## Q5) For speedup 10 on 64 cores, what is the maximum sequential part?

### Answer

We need $S(64) = 10$ with 64 cores.

**Using Amdahl's law:**

$$
S(64) = \frac{1}{T_s + \frac{1 - T_s}{64}} = 10
$$

So:

$$
\frac{1}{T_s + \frac{1 - T_s}{64}} = 10
$$

$$
T_s + \frac{1 - T_s}{64} = 0.1
$$

Multiply both sides by 64:

$$
64 T_s + (1 - T_s) = 6.4
$$

$$
64 T_s - T_s = 6.4 - 1
$$

$$
63 T_s = 5.4
$$

$$
T_s = \frac{5.4}{63} \approx 0.0857
$$

So the sequential part can be at most about **8.57%**.

**Final answer:** The sequential (unparallelizable) part must be at most **8.57%**.

**Note:** The $O(n^3)$ complexity tells us the algorithm is heavy (runtime grows fast with input size). The Amdahl bound above gives the maximum allowed sequential fraction to reach speedup 10 on 64 cores.
