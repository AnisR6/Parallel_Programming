#include <stdio.h>
#include <stdlib.h>
#include <xmmintrin.h>
#include <omp.h>

static void init_arrays(float *a, float *b, float *c, int size)
{
    for (int i = 0; i < size; ++i)
    {
        a[i] = 1.0f;
        b[i] = 2.0f;
        c[i] = 3.0f;
    }
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage: %s <size>\n", argv[0]);
        return 1;
    }

    int size = atoi(argv[1]);
    int repetitions = 1000000;

    /* Allocate 16-byte aligned memory required by _mm_load_ps / _mm_store_ps */
    float *a = (float *)aligned_alloc(16, size * sizeof(float));
    float *b = (float *)aligned_alloc(16, size * sizeof(float));
    float *c = (float *)aligned_alloc(16, size * sizeof(float));

    if (a == NULL || b == NULL || c == NULL)
    {
        printf("Memory allocation failed\n");
        return 1;
    }

    init_arrays(a, b, c, size);

    double start = omp_get_wtime();

    for (int run = 0; run < repetitions; ++run)
    {
        /* here we process 4 floats at a time with 128-bit XMM registers */

        for (int i = 0; i < size; i += 4)
        {
            __m128 va = _mm_load_ps(&a[i]);   /* load 4 floats from a */
            __m128 vb = _mm_load_ps(&b[i]);   /* load 4 floats from b */
            __m128 vc = _mm_load_ps(&c[i]);   /* load 4 floats from c */

            __m128 vmul = _mm_mul_ps(vb, vc); /* vb * vc */
            __m128 vadd = _mm_add_ps(va, vmul); /* va + (vb * vc) */

            _mm_store_ps(&a[i], vadd);         /* store result back */
        }
    }

    double end = omp_get_wtime();

    printf("a[0] = %f\n", a[0]);
    printf("a[last] = %f\n", a[size - 1]);
    printf("Computation time: %f seconds\n", end - start);

    free(a);
    free(b);
    free(c);

    return 0;
}
