#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

static void init_arrays(double *a, double *b, double *c, int size)
{
    for (int i = 0; i < size; ++i)
    {
        a[i] = 1.0;
        b[i] = 2.0;
        c[i] = 3.0;
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

    double *a = (double *)malloc(size * sizeof(double));
    double *b = (double *)malloc(size * sizeof(double));
    double *c = (double *)malloc(size * sizeof(double));

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
