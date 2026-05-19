#include <stdio.h>
#include <omp.h>

#define N 500
#define M 500
#define L 500

static double a_normal[N + 1][M][L];
static double a_parallel[N + 1][M][L];

// filling up arrays
void init_arrays() {
    for (int i = 0; i <= N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < L; k++) {
                a_normal[i][j][k] = i + j + k;
                a_parallel[i][j][k] = i + j + k;
            }
        }
    }
}

void normal_version() {
    for (int i = 1; i < N; i++) {
        for (int j = 1; j < M; j++) {
            for (int k = 1; k < L; k++) {
                a_normal[i + 1][j][k - 1] = a_normal[i][j][k] + 5;
            }
        }
    }
}

/* here we permutaed the loop of J with I , becase as we can see on the write and read ,
 we do not have dependencies on the same j as it writes to the same iteration, thats why we did pragma only on that specific loop */

void parallel_over_j_version() {
    #pragma omp parallel for
    for (int j = 1; j < M; j++) {
        for (int i = 1; i < N; i++) {
            for (int k = 1; k < L; k++) {
                a_parallel[i + 1][j][k - 1] = a_parallel[i][j][k] + 5;
            }
        }
    }
}

int main() {
    double start, end;

    init_arrays();

    start = omp_get_wtime();
    normal_version();
    end = omp_get_wtime();
    printf("Normal version time: %f seconds\n", end - start);

    start = omp_get_wtime();
    parallel_over_j_version();
    end = omp_get_wtime();
    printf("Parallel over j version time: %f seconds\n", end - start);

    printf("Example check:\n");
    printf("a_normal[10][10][10]   = %f\n", a_normal[10][10][10]);
    printf("a_parallel[10][10][10] = %f\n", a_parallel[10][10][10]);


    return 0;
}