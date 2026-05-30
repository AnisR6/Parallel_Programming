#include <stdio.h>
#include <stdlib.h>
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

    float *a = (float *)malloc(size * sizeof(float));
    float *b = (float *)malloc(size * sizeof(float));
    float *c = (float *)malloc(size * sizeof(float));

    if (a == NULL || b == NULL || c == NULL)
    {
        printf("Memory allocation failed\n");
        return 1;
    }

    init_arrays(a, b, c, size);

    double start = omp_get_wtime();

    for (int run = 0; run < repetitions; ++run)
    {
        #pragma omp simd
        for (int i = 0; i < size; ++i)
        {
            a[i] += b[i] * c[i];
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
