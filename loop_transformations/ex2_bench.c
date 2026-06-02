#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define N          (1 << 22)   /* 4 M elements */
#define MAT_N      512         /* matrix size for tiling (512×512 doubles) */
#define BLOCK_SIZE (64 / sizeof(double))  /* = 8 */
#define REPS       5

static double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

/* prevent dead-code elimination */
static volatile long   sink_i;
static volatile double sink_d;

/* ------------------------------------------------------------------ */
/* 1 – Loop Unrolling                                                  */
/* ------------------------------------------------------------------ */
static void bench_1(void) {
    int *a = malloc(N * sizeof(int));
    int *b = malloc(N * sizeof(int));
    for (int i = 0; i < N; ++i) b[i] = i;

    double t0, t1;

    t0 = now_sec();
    for (int r = 0; r < REPS; ++r)
        for (int i = 0; i < N - 1; ++i)
            a[i] = b[i] + b[i + 1];
    t1 = now_sec();
    sink_i = a[0];
    printf("[1] original  (no unroll):        %.4f s\n", t1 - t0);

    t0 = now_sec();
    for (int r = 0; r < REPS; ++r) {
        int i;
        for (i = 0; i < N - 2; i += 2) {
            a[i]     = b[i]     + b[i + 1];
            a[i + 1] = b[i + 1] + b[i + 2];
        }
    }
    t1 = now_sec();
    sink_i = a[0];
    printf("[1] optimised (unroll x2):        %.4f s\n\n", t1 - t0);

    free(a); free(b);
}

/* ------------------------------------------------------------------ */
/* 2 – Loop-Invariant Code Motion                                      */
/* ------------------------------------------------------------------ */
static void bench_2(void) {
    double *a = malloc(N * sizeof(double));
    for (int i = 0; i < N; ++i) a[i] = (double)i;

    double t0, t1;

    t0 = now_sec();
    for (int r = 0; r < REPS; ++r)
        for (int i = 0; i < N; ++i)
            a[i] *= hypot(0.3, 0.4);
    t1 = now_sec();
    sink_d = a[0];
    printf("[2] original  (hypot in loop):    %.4f s\n", t1 - t0);

    for (int i = 0; i < N; ++i) a[i] = (double)i;

    t0 = now_sec();
    for (int r = 0; r < REPS; ++r) {
        double h = hypot(0.3, 0.4);
        for (int i = 0; i < N; ++i)
            a[i] *= h;
    }
    t1 = now_sec();
    sink_d = a[0];
    printf("[2] optimised (hypot hoisted):    %.4f s\n\n", t1 - t0);

    free(a);
}

/* ------------------------------------------------------------------ */
/* 3 – Loop Unswitching                                                */
/* ------------------------------------------------------------------ */
static void bench_3(void) {
    int *a = malloc(N * sizeof(int));
    int *b = malloc(N * sizeof(int));
    int *c = malloc(N * sizeof(int));
    for (int i = 0; i < N; ++i) { b[i] = i; c[i] = i * 2; }

    double t0, t1;

    t0 = now_sec();
    for (int r = 0; r < REPS; ++r)
        for (int i = 0; i < N; ++i) {
            if (N % 2) a[i] = b[i] + 5;
            else       a[i] = c[i] + 5;
        }
    t1 = now_sec();
    sink_i = a[0];
    printf("[3] original  (branch in loop):   %.4f s\n", t1 - t0);

    t0 = now_sec();
    for (int r = 0; r < REPS; ++r) {
        if (N % 2)
            for (int i = 0; i < N; ++i) a[i] = b[i] + 5;
        else
            for (int i = 0; i < N; ++i) a[i] = c[i] + 5;
    }
    t1 = now_sec();
    sink_i = a[0];
    printf("[3] optimised (unswitched):       %.4f s\n\n", t1 - t0);

    free(a); free(b); free(c);
}

/* ------------------------------------------------------------------ */
/* 4 – Loop Fission                                                    */
/* ------------------------------------------------------------------ */
static void bench_4(void) {
    int *a = malloc(N * sizeof(int));
    int *b = malloc(N * sizeof(int));
    int *c = malloc(N * sizeof(int));
    for (int i = 0; i < N; ++i) { a[i] = i; b[i] = i+1; c[i] = i+2; }

    long sum_a, sum_b, sum_c;
    double t0, t1;

    t0 = now_sec();
    for (int r = 0; r < REPS; ++r) {
        sum_a = sum_b = sum_c = 0;
        for (int i = 0; i < N; ++i) {
            sum_a += a[i];
            sum_b += b[i];
            sum_c += c[i];
        }
    }
    t1 = now_sec();
    sink_i = sum_a + sum_b + sum_c;
    printf("[4] original  (fused loop):       %.4f s\n", t1 - t0);

    t0 = now_sec();
    for (int r = 0; r < REPS; ++r) {
        sum_a = sum_b = sum_c = 0;
        for (int i = 0; i < N; ++i) sum_a += a[i];
        for (int i = 0; i < N; ++i) sum_b += b[i];
        for (int i = 0; i < N; ++i) sum_c += c[i];
    }
    t1 = now_sec();
    sink_i = sum_a + sum_b + sum_c;
    printf("[4] optimised (fissioned):        %.4f s\n\n", t1 - t0);

    free(a); free(b); free(c);
}

/* ------------------------------------------------------------------ */
/* 5 – Loop Peeling + Fusion                                           */
/* ------------------------------------------------------------------ */
static void bench_5(void) {
    int *a = malloc(N * sizeof(int));
    for (int i = 0; i < N; ++i) a[i] = rand() % 1000;

    long sum; int mn;
    double t0, t1;

    t0 = now_sec();
    for (int r = 0; r < REPS; ++r) {
        mn = a[0];
        for (int i = 1; i < N; ++i)
            mn = (a[i] < mn) ? a[i] : mn;
        sum = 0;
        for (int i = 0; i < N; ++i)
            sum += a[i];
    }
    t1 = now_sec();
    sink_i = sum + mn;
    printf("[5] original  (two loops):        %.4f s\n", t1 - t0);

    t0 = now_sec();
    for (int r = 0; r < REPS; ++r) {
        mn = a[0]; sum = a[0];
        for (int i = 1; i < N; ++i) {
            if (a[i] < mn) mn = a[i];
            sum += a[i];
        }
    }
    t1 = now_sec();
    sink_i = sum + mn;
    printf("[5] optimised (peeled+fused):     %.4f s\n\n", t1 - t0);

    free(a);
}

/* ------------------------------------------------------------------ */
/* 6 – Loop Splitting                                                  */
/* ------------------------------------------------------------------ */
static void bench_6(void) {
    int *a = malloc(N * sizeof(int));
    int *b = malloc(N * sizeof(int));
    int *c = malloc(N * sizeof(int));
    for (int i = 0; i < N; ++i) { b[i] = i; c[i] = i * 2; }

    double t0, t1;

    t0 = now_sec();
    for (int r = 0; r < REPS; ++r)
        for (int i = 0; i < N; ++i) {
            if (i % 2) a[i] = b[i] + 4;
            else       a[i] = c[i] + 5;
        }
    t1 = now_sec();
    sink_i = a[0];
    printf("[6] original  (branch on i%%2):    %.4f s\n", t1 - t0);

    t0 = now_sec();
    for (int r = 0; r < REPS; ++r) {
        for (int i = 0; i < N; i += 2) a[i]     = c[i] + 5;
        for (int i = 1; i < N; i += 2) a[i]     = b[i] + 4;
    }
    t1 = now_sec();
    sink_i = a[0];
    printf("[6] optimised (split even/odd):   %.4f s\n\n", t1 - t0);

    free(a); free(b); free(c);
}

/* ------------------------------------------------------------------ */
/* 7 – Loop Tiling                                                     */
/* ------------------------------------------------------------------ */
static void bench_7(void) {
    int n = MAT_N;
    double (*a)[MAT_N] = malloc(n * n * sizeof(double));
    double (*b)[MAT_N] = malloc(n * n * sizeof(double));
    double (*c)[MAT_N] = malloc(n * n * sizeof(double));

    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j) {
            a[i][j] = (double)(i + j);
            b[i][j] = (double)(i - j);
            c[i][j] = 0.0;
        }

    double t0, t1;

    /* original */
    memset(c, 0, n * n * sizeof(double));
    t0 = now_sec();
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            for (int k = 0; k < n; ++k)
                c[i][j] += a[i][k] * b[k][j];
    t1 = now_sec();
    sink_d = c[0][0];
    printf("[7] original  (naive matmul):     %.4f s\n", t1 - t0);

    /* tiled */
    memset(c, 0, n * n * sizeof(double));
    t0 = now_sec();
    for (int ii = 0; ii < n; ii += BLOCK_SIZE)
      for (int jj = 0; jj < n; jj += BLOCK_SIZE)
        for (int kk = 0; kk < n; kk += BLOCK_SIZE)
          for (int i = ii; i < ii + BLOCK_SIZE && i < n; ++i)
            for (int j = jj; j < jj + BLOCK_SIZE && j < n; ++j)
              for (int k = kk; k < kk + BLOCK_SIZE && k < n; ++k)
                c[i][j] += a[i][k] * b[k][j];
    t1 = now_sec();
    sink_d = c[0][0];
    printf("[7] optimised (tiled %dx%d):     %.4f s\n\n",
           (int)BLOCK_SIZE, (int)BLOCK_SIZE, t1 - t0);

    free(a); free(b); free(c);
}

/* ------------------------------------------------------------------ */
int main(void) {
    printf("=== Exercise 2 – Loop Optimization Benchmarks ===\n");
    printf("N = %d, MAT_N = %d, REPS = %d\n\n", N, MAT_N, REPS);

    bench_1();
    bench_2();
    bench_3();
    bench_4();
    bench_5();
    bench_6();
    bench_7();

    return 0;
}
