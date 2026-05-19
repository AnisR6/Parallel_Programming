#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Returns 1 if the environment variable FIRST_TOUCH_BAD is set to "1" */
/* The only code change between case 1 and case 2 is whether the init loop has
#pragma`.we control this with an environment variable, no recompile is needed */

static int bad_first_touch(void) {
    const char *v = getenv("FIRST_TOUCH_BAD");
    return (v && v[0] == '1');
}



int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: %s <N>\n", argv[0]);
        return EXIT_FAILURE;
    }
    int N = atoi(argv[1]);

    omp_set_num_threads(12);

    int bad = bad_first_touch();
    printf("=== first_touch  N=%d  threads=12  mode=%s ===\n",
           N, bad ? "BAD (serial init)" : "GOOD (parallel init)");

    long long **matrix;
    double t0, t1;



    /* Step 1 : Allocate (serial, malloc only reserves virtual memory) */



    t0 = omp_get_wtime();
    matrix = (long long **)malloc(N * sizeof(long long *));
    for (int i = 0; i < N; i++)
        matrix[i] = (long long *)malloc(N * sizeof(long long));
    t1 = omp_get_wtime();
    printf("Memory allocation time  : %.6f s\n", t1 - t0);




    /* Step 2 : Initialise (GOOD = parallel, BAD = serial) */


    t0 = omp_get_wtime();
    if (bad) {

        /* Serial: master thread writes everything → all pages on one chip */

        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++)
                matrix[i][j] = (long long)(i + j);
    } else {
        
        /* Parallel: each thread writes its own rows → pages spread across chips */

#pragma omp parallel for schedule(static)
        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++)
                matrix[i][j] = (long long)(i + j);
    }
    t1 = omp_get_wtime();
    printf("Matrix initialisation   : %.6f s\n", t1 - t0);

    /* Step 3 : Compute (always parallel, same schedule as init) */


    long long sum = 0;
    t0 = omp_get_wtime();
#pragma omp parallel for schedule(static) reduction(+:sum)
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            sum += matrix[i][j];
    t1 = omp_get_wtime();
    printf("Matrix computation time : %.6f s\n", t1 - t0);
    printf("Sum of matrix elements  : %lld\n", sum);

    

    /* Step 4 : Free (parallel per row, then serial for the pointer array) */


    t0 = omp_get_wtime();
#pragma omp parallel for schedule(static)
    for (int i = 0; i < N; i++)
        free(matrix[i]);
    free(matrix);
    t1 = omp_get_wtime();
    printf("Memory deallocation time: %.6f s\n", t1 - t0);

    return EXIT_SUCCESS;
}