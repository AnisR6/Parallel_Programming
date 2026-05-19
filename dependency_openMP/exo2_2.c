#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1000000

void original_loop(int *A) {
    for (int i = 4; i <= N; i += 9) {
        for (int j = 0; j <= N; j += 5) {
            A[i] = 0;
        }
    }
}

/* here we normalised it by substracting the start indice, and then dividing by the number of steps*/

void normalized_loop(int *A) {
    for (int ii = 0; ii <= (N - 4) / 9; ii++) {
        int i = 4 + 9 * ii;

        for (int jj = 0; jj <= N / 5; jj++) {
            int j = 5 * jj;

            A[i] = 0;
        }
    }
}

void normalized_parallel_loop(int *A) {
    #pragma omp parallel for
    for (int ii = 0; ii <= (N - 4) / 9; ii++) {
        int i = 4 + 9 * ii;

        for (int jj = 0; jj <= N / 5; jj++) {
            int j = 5 * jj;

            A[i] = 0;
        }
    }
}

// here for the simplified, i just removed the loop j as we did not use it on the iterations of the writing on A as i mentioned on the presentation

void simplified_parallel_loop(int *A) {
    #pragma omp parallel for
    for (int ii = 0; ii <= (N - 4) / 9; ii++) {
        int i = 4 + 9 * ii;
        A[i] = 0;
    }
}

int main() {
    int *A = malloc((N + 1) * sizeof(int));

    if (A == NULL) {
        printf("Memory allocation failed\n");
        return 1;
    }

    for (int i = 0; i <= N; i++) {
        A[i] = 1;
    }

    double t1 = omp_get_wtime();
    original_loop(A);
    double t2 = omp_get_wtime();
    printf("Original loop time: %f seconds\n", t2 - t1);

    for (int i = 0; i <= N; i++) {
        A[i] = 1;
    }

    t1 = omp_get_wtime();
    normalized_loop(A);
    t2 = omp_get_wtime();
    printf("Normalized loop time: %f seconds\n", t2 - t1);

    for (int i = 0; i <= N; i++) {
        A[i] = 1;
    }

    t1 = omp_get_wtime();
    normalized_parallel_loop(A);
    t2 = omp_get_wtime();
    printf("Normalized parallel loop time: %f seconds\n", t2 - t1);

    for (int i = 0; i <= N; i++) {
        A[i] = 1;
    }

    t1 = omp_get_wtime();
    simplified_parallel_loop(A);
    t2 = omp_get_wtime();

    printf("Simplified parallel loop time: %f seconds\n", t2 - t1);

    free(A);

    return 0;
}