# Exercise 1 — Performance Profiling



## steps

```bash
# 1. Compile with profiling instrumentation
gcc -O3 -g3 -pg *.c -o real_prof -lm

# 2. Run normally and produces gmon.out automatically
./real_prof

# 3. Generate the report
gprof real_prof gmon.out > analysis.txt
```

## What information does the profile hold?

The profiling output consists of two parts:

**The flat profile** lists every function with its share of total runtime, its self time (time spent inside the function body only), the total time including callees, and how many times it was called.

**The call graph** shows the parent/child relationships between functions , who calls whom, and how time propagates up and down the call tree.

## What does the profile tell us?

Total runtime: **6.96 s**

### Flat profile

| % Time | Self (s) | Total (ms/call) | Calls  | Function   |
|-------:|---------:|----------------:|-------:|------------|
| 42.83  | 2.98     | 23.33           | 147    | `resid`    |
| 20.84  | 1.45     | 11.27           | 168    | `psinv`    |
| 14.08  | 0.98     | 0.01            | 131072 | `vranlc`   |
|  9.63  | 0.67     | 7.20            | 147    | `rprj3`    |
|  8.34  | 0.58     | 3.95            | 147    | `interp`   |
|  4.31  | 0.30     | 2.64            | 485    | `norm2u3`  |
|  0.00  | 0.00     | —               | 131642 | `randlc`   |
|  0.00  | 0.00     | —               | 4      | `wtime_`   |

### Call graph

`mg3P` sits at the root with 100% of total time but **zero self time** . it only calls to other functions and does no work itself. The hierarchy is:

- `mg3P` calls `resid`, `psinv`, `rprj3`, `interp` directly
- `resid`, `psinv`, and `rprj3` each call `norm2u3`
- `norm2u3` calls `vranlc` (131072 times) and `randlc` (131642 times)
- `interp` and `randlc` are pure leaf functions — they call nothing

`norm2u3` has only 0.30s of self time but accounts for 18.4% of total time because it drags in `vranlc` (0.98s) as a child. Its true cost is therefore higher than the flat profile alone suggests.

## Why is this useful?

Without profiling, one might attempt to parallelize `mg3P` ,  but the call graph immediately shows it has zero self time, making it a pointless target. The profile instead points directly to `resid`, `psinv`, `rprj3`, and `interp` as the four functions worth parallelizing, together accounting for over **80% of total runtime**. The call graph also reveals that `vranlc`, despite appearing significant in the flat profile at 14%, is a deep callee of `norm2u3` which itself is called from within the main computation functions and its cost is already accounted for in their total times.

In short, the profile replaces guesswork with hard numbers, showing exactly where time is spent and how functions relate to each other.
