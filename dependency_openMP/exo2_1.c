#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1024

void copy_normal(double *x, double *y) {
    for (int i = 0; i < N; i++) {
        x[i] = y[i];
    }
}

// here adding a parralel version with openmp so we can observe the performance 

void copy_parallel(double *x, double *y) {
    #pragma omp parallel for
    for (int i = 0; i < N; i++) {
        x[i] = y[i];
    }
}

int main() {
    double *x = malloc(N * sizeof(double));
    double *y = malloc(N * sizeof(double));

    if (x == NULL || y == NULL) {
        printf("Memory allocation failed\n");
        return 1;
    }

// filling up the arrays

    for (int i = 0; i < N; i++) {
        y[i] = i * 1.0;
        x[i] = 0.0;
    }

    double t1 = omp_get_wtime();
    copy_normal(x, y);
    double t2 = omp_get_wtime();

    printf("Normal copy versiontime: %f seconds\n", t2 - t1);

    for (int i = 0; i < N; i++) {
        x[i] = 0.0;
    }

    t1 = omp_get_wtime();
    copy_parallel(x, y);
    t2 = omp_get_wtime();

    printf("enhanced copy version time: %f seconds\n", t2 - t1);

    printf("Check: x[10] = %f, y[10] = %f\n", x[10], y[10]);

    free(x);
    free(y);

    return 0;
}