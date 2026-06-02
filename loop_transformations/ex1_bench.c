#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

/* ------------------------------------------------------------------ */
/*  Helpers                                                            */
/* ------------------------------------------------------------------ */
#define N        (1 << 24)   /* 16 M iterations – large enough to measure */
#define REPS     10          /* repeat each benchmark for stable timing    */

static double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

/* Prevent the compiler from optimising away "dead" results */
static volatile unsigned  sink_u;
static volatile float     sink_f;
static volatile double    sink_d;

/* ------------------------------------------------------------------ */
/*  a)  32 * c1                                                        */
/* ------------------------------------------------------------------ */
static void bench_a(void) {
    double t0, t1;

    /* --- original --- */
    t0 = now_sec();
    for (int r = 0; r < REPS; ++r) {
        for (unsigned c1 = 0; c1 < N; ++c1) {
            unsigned c2 = 32 * c1;
            sink_u = c2;
        }
    }
    t1 = now_sec();
    printf("[a] original  (32 * c1):   %.4f s\n", t1 - t0);

    /* --- optimised --- */
    t0 = now_sec();
    for (int r = 0; r < REPS; ++r) {
        for (unsigned c1 = 0; c1 < N; ++c1) {
            unsigned c2 = c1 << 5;
            sink_u = c2;
        }
    }
    t1 = now_sec();
    printf("[a] optimised (c1 << 5):   %.4f s\n\n", t1 - t0);
}

/* ------------------------------------------------------------------ */
/*  b)  15 * c1                                                        */
/* ------------------------------------------------------------------ */
static void bench_b(void) {
    double t0, t1;

    t0 = now_sec();
    for (int r = 0; r < REPS; ++r) {
        for (unsigned c1 = 0; c1 < N; ++c1) {
            unsigned c2 = 15 * c1;
            sink_u = c2;
        }
    }
    t1 = now_sec();
    printf("[b] original  (15 * c1):         %.4f s\n", t1 - t0);

    t0 = now_sec();
    for (int r = 0; r < REPS; ++r) {
        for (unsigned c1 = 0; c1 < N; ++c1) {
            unsigned c2 = (c1 << 4) - c1;   /* 16*c1 - c1 = 15*c1 */
            sink_u = c2;
        }
    }
    t1 = now_sec();
    printf("[b] optimised ((c1<<4)-c1):       %.4f s\n\n", t1 - t0);
}

/* ------------------------------------------------------------------ */
/*  c)  96 * c1                                                        */
/* ------------------------------------------------------------------ */
static void bench_c(void) {
    double t0, t1;

    t0 = now_sec();
    for (int r = 0; r < REPS; ++r) {
        for (unsigned c1 = 0; c1 < N; ++c1) {
            unsigned c2 = 96 * c1;
            sink_u = c2;
        }
    }
    t1 = now_sec();
    printf("[c] original  (96 * c1):              %.4f s\n", t1 - t0);

    t0 = now_sec();
    for (int r = 0; r < REPS; ++r) {
        for (unsigned c1 = 0; c1 < N; ++c1) {
            unsigned c2 = (c1 << 6) + (c1 << 5);  /* 64*c1 + 32*c1 */
            sink_u = c2;
        }
    }
    t1 = now_sec();
    printf("[c] optimised ((c1<<6)+(c1<<5)):      %.4f s\n\n", t1 - t0);
}

/* ------------------------------------------------------------------ */
/*  d)  0.125 * c1                                                     */
/* ------------------------------------------------------------------ */
static void bench_d(void) {
    double t0, t1;

    t0 = now_sec();
    for (int r = 0; r < REPS; ++r) {
        for (unsigned c1 = 0; c1 < N; ++c1) {
            unsigned c2 = (unsigned)(0.125 * c1);
            sink_u = c2;
        }
    }
    t1 = now_sec();
    printf("[d] original  (0.125 * c1):   %.4f s\n", t1 - t0);

    t0 = now_sec();
    for (int r = 0; r < REPS; ++r) {
        for (unsigned c1 = 0; c1 < N; ++c1) {
            unsigned c2 = c1 >> 3;           /* logical right-shift by 3 */
            sink_u = c2;
        }
    }
    t1 = now_sec();
    printf("[d] optimised (c1 >> 3):      %.4f s\n\n", t1 - t0);
}

/* ------------------------------------------------------------------ */
/*  e)  Stride-5 loop with 5*i                                         */
/* ------------------------------------------------------------------ */
static void bench_e(void) {
    unsigned *a = malloc(N * sizeof(unsigned));
    if (!a) { perror("malloc"); exit(1); }
    for (int i = 0; i < N; ++i) a[i] = (unsigned)i;

    double t0, t1;

    /* --- original: multiply inside loop --- */
    t0 = now_sec();
    for (int r = 0; r < REPS; ++r) {
        unsigned sum = 0;
        for (int i = 0; i < N / 5; ++i)
            sum += a[5 * i];
        sink_u = sum;
    }
    t1 = now_sec();
    printf("[e] original  (5 * i):         %.4f s\n", t1 - t0);

    /* --- optimised: induction variable --- */
    t0 = now_sec();
    for (int r = 0; r < REPS; ++r) {
        unsigned sum = 0;
        for (int i = 0; i < N; i += 5)
            sum += a[i];
        sink_u = sum;
    }
    t1 = now_sec();
    printf("[e] optimised (i += 5):        %.4f s\n\n", t1 - t0);

    free(a);
}

/* ------------------------------------------------------------------ */
/*  f)  i / 5.3  vs  i * (1.0/5.3)                                    */
/* ------------------------------------------------------------------ */
static void bench_f(void) {
    double *a = malloc(N * sizeof(double));
    if (!a) { perror("malloc"); exit(1); }
    memset(a, 0, N * sizeof(double));

    double t0, t1;

    /* --- original: divide inside loop --- */
    t0 = now_sec();
    for (int r = 0; r < REPS; ++r) {
        for (int i = 0; i < N; ++i)
            a[i] += i / 5.3;
        sink_d = a[0];
    }
    t1 = now_sec();
    printf("[f] original  (i / 5.3):             %.4f s\n", t1 - t0);

    memset(a, 0, N * sizeof(double));

    /* --- optimised: precompute reciprocal --- */
    t0 = now_sec();
    for (int r = 0; r < REPS; ++r) {
        const double inv = 1.0 / 5.3;
        for (int i = 0; i < N; ++i)
            a[i] += i * inv;
        sink_d = a[0];
    }
    t1 = now_sec();
    printf("[f] optimised (i * (1.0/5.3)):        %.4f s\n\n", t1 - t0);

    free(a);
}

/* ------------------------------------------------------------------ */
/*  g)  -1 * c1  vs  -c1  (float sign flip)                           */
/* ------------------------------------------------------------------ */
static void bench_g(void) {
    double t0, t1;

    t0 = now_sec();
    for (int r = 0; r < REPS; ++r) {
        for (int i = 0; i < N; ++i) {
            float c1 = (float)i;
            float c2 = -1.0f * c1;   /* float multiply */
            sink_f = c2;
        }
    }
    t1 = now_sec();
    printf("[g] original  (-1 * c1):     %.4f s\n", t1 - t0);

    t0 = now_sec();
    for (int r = 0; r < REPS; ++r) {
        for (int i = 0; i < N; ++i) {
            float c1 = (float)i;
            float c2 = -c1;          /* sign-bit flip / fneg */
            sink_f = c2;
        }
    }
    t1 = now_sec();
    printf("[g] optimised (-c1):         %.4f s\n\n", t1 - t0);
}

/* ------------------------------------------------------------------ */
/*  main                                                               */
/* ------------------------------------------------------------------ */
int main(void) {
    printf("=== Exercise 1 – Strength Reduction Benchmarks ===\n");
    printf("N = %d, REPS = %d\n\n", N, REPS);

    bench_a();
    bench_b();
    bench_c();
    bench_d();
    bench_e();
    bench_f();
    bench_g();

    return 0;
}
