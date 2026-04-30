#define _POSIX_C_SOURCE 200809L

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define X 1280
#define Y 720
#define MAX_ITER 10000

static uint8_t image[Y][X];

static double monotonic_seconds(void) {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        perror("clock_gettime");
        exit(EXIT_FAILURE);
    }
    return (double)ts.tv_sec + 1e-9 * (double)ts.tv_nsec;
}

/* schedule_str is passed straight to the label, e.g. "static", "dynamic,1", etc. */
static double run(const char *schedule_label,
                  void (*compute)(void)) {
    double t0 = monotonic_seconds();
    compute();
    double elapsed = monotonic_seconds() - t0;
    printf("  %-20s  %.4f s\n", schedule_label, elapsed);
    return elapsed;
}

/* ---------- six separate compute functions, one per schedule ---------- */

static void compute_static(void) {
#pragma omp parallel for schedule(static)
    for (int y_pixel = 0; y_pixel < Y; y_pixel++) {
        for (int x_pixel = 0; x_pixel < X; x_pixel++) {
            double x = 0.0, y = 0.0;
            double cx = -2.5 + (x_pixel / (double)(X - 1)) * 3.5;
            double cy =  1.0 - (y_pixel / (double)(Y - 1)) * 2.0;
            int iter = 0;
            while (x*x + y*y <= 4.0 && iter < MAX_ITER) {
                double xt = x*x - y*y + cx;
                y = 2.0*x*y + cy;
                x = xt;
                iter++;
            }
            image[y_pixel][x_pixel] = (uint8_t)((iter * 255) / MAX_ITER);
        }
    }
}

static void compute_dynamic(void) {
#pragma omp parallel for schedule(dynamic, 1)
    for (int y_pixel = 0; y_pixel < Y; y_pixel++) {
        for (int x_pixel = 0; x_pixel < X; x_pixel++) {
            double x = 0.0, y = 0.0;
            double cx = -2.5 + (x_pixel / (double)(X - 1)) * 3.5;
            double cy =  1.0 - (y_pixel / (double)(Y - 1)) * 2.0;
            int iter = 0;
            while (x*x + y*y <= 4.0 && iter < MAX_ITER) {
                double xt = x*x - y*y + cx;
                y = 2.0*x*y + cy;
                x = xt;
                iter++;
            }
            image[y_pixel][x_pixel] = (uint8_t)((iter * 255) / MAX_ITER);
        }
    }
}

static void compute_guided(void) {
#pragma omp parallel for schedule(guided)
    for (int y_pixel = 0; y_pixel < Y; y_pixel++) {
        for (int x_pixel = 0; x_pixel < X; x_pixel++) {
            double x = 0.0, y = 0.0;
            double cx = -2.5 + (x_pixel / (double)(X - 1)) * 3.5;
            double cy =  1.0 - (y_pixel / (double)(Y - 1)) * 2.0;
            int iter = 0;
            while (x*x + y*y <= 4.0 && iter < MAX_ITER) {
                double xt = x*x - y*y + cx;
                y = 2.0*x*y + cy;
                x = xt;
                iter++;
            }
            image[y_pixel][x_pixel] = (uint8_t)((iter * 255) / MAX_ITER);
        }
    }
}

static void compute_auto(void) {
#pragma omp parallel for schedule(auto)
    for (int y_pixel = 0; y_pixel < Y; y_pixel++) {
        for (int x_pixel = 0; x_pixel < X; x_pixel++) {
            double x = 0.0, y = 0.0;
            double cx = -2.5 + (x_pixel / (double)(X - 1)) * 3.5;
            double cy =  1.0 - (y_pixel / (double)(Y - 1)) * 2.0;
            int iter = 0;
            while (x*x + y*y <= 4.0 && iter < MAX_ITER) {
                double xt = x*x - y*y + cx;
                y = 2.0*x*y + cy;
                x = xt;
                iter++;
            }
            image[y_pixel][x_pixel] = (uint8_t)((iter * 255) / MAX_ITER);
        }
    }
}

static void compute_runtime(void) {
#pragma omp parallel for schedule(runtime)
    for (int y_pixel = 0; y_pixel < Y; y_pixel++) {
        for (int x_pixel = 0; x_pixel < X; x_pixel++) {
            double x = 0.0, y = 0.0;
            double cx = -2.5 + (x_pixel / (double)(X - 1)) * 3.5;
            double cy =  1.0 - (y_pixel / (double)(Y - 1)) * 2.0;
            int iter = 0;
            while (x*x + y*y <= 4.0 && iter < MAX_ITER) {
                double xt = x*x - y*y + cx;
                y = 2.0*x*y + cy;
                x = xt;
                iter++;
            }
            image[y_pixel][x_pixel] = (uint8_t)((iter * 255) / MAX_ITER);
        }
    }
}

int main(int argc, char **argv) {
    int num_threads = 1;
    if (argc >= 2) {
        num_threads = atoi(argv[1]);
        if (num_threads < 1) num_threads = 1;
    }
    omp_set_num_threads(num_threads);

    printf("Threads: %d\n", num_threads);

    /* Thread-count benchmark (static schedule, vary threads) */
    if (argc == 2) {
        run("static", compute_static);
        stbi_write_png("mandelbrot.png", X, Y, 1, image, 0);
        return EXIT_SUCCESS;
    }

    /* Scheduling comparison (called with argv[2] == "schedules") */
    printf("%-22s  %s\n", "Schedule", "Time");
    printf("--------------------------------------\n");
    run("static",        compute_static);
    run("dynamic,1",     compute_dynamic);
    run("guided",        compute_guided);
    run("auto",          compute_auto);
    run("runtime",       compute_runtime);

    stbi_write_png("mandelbrot.png", X, Y, 1, image, 0);
    return EXIT_SUCCESS;
}
