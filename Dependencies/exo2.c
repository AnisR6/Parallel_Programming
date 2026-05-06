#include <stdio.h>
#include <math.h>
#include <time.h>
#include <omp.h>
#include <stdlib.h>

#define N 100000000
double *x_a, *y_a;
double *x_b, *y_b, *z_b;
double *x_c, *y_c;

/* ── Timing ────────────────────────────────────── */
double get_wall_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

void benchmark(const char *label, void (*fn)(void)) {
    double t0 = get_wall_time();
    fn();
    printf("%s: %.4f s\n", label, get_wall_time() - t0);
}

/* ── (a) ─────────────────────────────────────────────────────── */
void loop_a_seq() {
    double factor = 1.0;
    for (int i = 0; i < N; i++) {
        x_a[i] = factor * y_a[i];
        factor /= 2.0;
    }
}

void loop_a_par() {
    #pragma omp parallel for
    for (int i = 0; i < N; i++) {
        x_a[i] = pow(0.5, i) * y_a[i];
    }
}

/* ── (b) ──────────────────────────────────────────────────────────── */

void loop_b_seq() {
    for (int i = 1; i < N; i++) {
        x_b[i] = (x_b[i] + y_b[i-1]) / 2;
        y_b[i] = y_b[i] + z_b[i] * 3;
    }
}

void loop_b_par() {
    /* First loop uses original y values */
    #pragma omp parallel for
    for (int i = 1; i < N; i++) {
        x_b[i] = (x_b[i] + y_b[i-1]) / 2;
    }

    /* Second loop depends only on z[i] */
    #pragma omp parallel for
    for (int i = 1; i < N; i++) {
        y_b[i] = y_b[i] + z_b[i] * 3;
    }
}

/* ── (c)  ────────────── */

void loop_c_seq() {
    x_c[0] = x_c[0] + 5 * y_c[0];
    for (int i = 1; i < N; i++) {
        x_c[i] = x_c[i] + 5 * y_c[i];
        x_c[i-1] = 2 * x_c[i-1];
    }
}

void loop_c_par() {
    #pragma omp parallel for
    for (int i = 0; i < N - 1; i++) {
        x_c[i] = 2 * (x_c[i] + 5 * y_c[i]);
    }
    x_c[N-1] = x_c[N-1] + 5 * y_c[N-1];
}

int main() {


    /* Allocate on heap — avoids 32-bit relocation overflow on large globals */

    x_a = malloc(N * sizeof(double));
    y_a = malloc(N * sizeof(double));
    x_b = malloc(N * sizeof(double));
    y_b = malloc(N * sizeof(double));
    z_b = malloc(N * sizeof(double));
    x_c = malloc(N * sizeof(double));
    y_c = malloc(N * sizeof(double));

    if (!x_a || !y_a || !x_b || !y_b || !z_b || !x_c || !y_c) {
        fprintf(stderr, "malloc failed\n");
        return 1;
    }

    for (int i = 0; i < N; i++) {
        y_a[i] = (double)i;
        x_a[i] = 0.0;

        x_b[i] = 0.0;
        y_b[i] = (double)i;
        z_b[i] = 1.0;

        x_c[i] = 0.0;
        y_c[i] = (double)i;
    }

    printf("Threads: %d\n\n", omp_get_max_threads());

    benchmark("(a) sequential", loop_a_seq);
    benchmark("(a) parallel  ", loop_a_par);
    printf("\n");
    benchmark("(b) sequential", loop_b_seq);
    benchmark("(b) parallel  ", loop_b_par);
    printf("\n");
    benchmark("(c) sequential", loop_c_seq);
    benchmark("(c) parallel  ", loop_c_par);

    free(x_a); free(y_a);
    free(x_b); free(y_b); free(z_b);
    free(x_c); free(y_c);

    return 0;
}