# Assignment 10 – Exercise 3: SSE Intrinsics

## Implementation

Used `_mm_load_ps`, `_mm_mul_ps`, `_mm_add_ps`, `_mm_store_ps` from `xmmintrin.h`.

Arrays allocated with `aligned_alloc(16, …)` ,  required by the load/store intrinsics.  
Inner loop processes **4 floats per iteration** using 128-bit XMM registers.

```c
for (int i = 0; i < size; i += 4) {
    __m128 va = _mm_load_ps(&a[i]);
    __m128 vb = _mm_load_ps(&b[i]);
    __m128 vc = _mm_load_ps(&c[i]);
    _mm_store_ps(&a[i], _mm_add_ps(va, _mm_mul_ps(vb, vc)));
}
```

Compiled with: `gcc -O1 -fopenmp -msse2 ex3_intrinsics.c -o vector_intrinsics`

---

## Results

| Size | Sequential (s) | Auto-vec Ex1 (s) | OMP SIMD Ex2 (s) | **Intrinsics Ex3 (s)** | Speedup vs seq |
|---:|---:|---:|---:|---:|---:|
| 128  | 0.171564 | 0.032962 | 0.032663 | 0.034509 | 4.97× |
| 256  | 0.339048 | 0.065547 | 0.064146 | 0.065867 | 5.15× |
| 512  | 0.686760 | 0.131807 | 0.133836 | 0.173805 | 3.95× |
| 1024 | 1.347385 | 0.261152 | 0.259656 | 0.343871 | 3.92× |
| 2048 | 2.691216 | 0.517917 | 0.511844 | 0.677756 | 3.97× |

Result is correct in all cases: `a[0] = a[last] = 6000001.000000`

---

## perf (size 2048)

```
r1010 (packed float ops): 1,024,009,119
```

---

## Observations

At sizes 128 and 256 all three methods perform similarly. From size 512 onward the intrinsics are slightly slower than Ex1 and Ex2, reaching only ~4× speedup compared to ~5.2×. This is because at `-O1` the compiler has less freedom to optimize around raw intrinsic calls. The result is still correct in all cases.

**Advantage:** you have full control over which instructions are used.

**Disadvantage:** the code is harder to read, only works on x86, requires manual memory alignment, and in this case ends up slightly slower than letting the compiler handle it.