#define _POSIX_C_SOURCE 200809L

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Include that allows to print result as an image
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Default size of image
#define X 1280
#define Y 720
#define MAX_ITER 10000

void calc_mandelbrot(uint8_t image[Y][X]) {
	
	for (int y_pixel = 0; y_pixel < Y; y_pixel++) {   
		for (int x_pixel = 0; x_pixel < X; x_pixel++) { 
			double x = 0.0;
			double y = 0.0;
			double cx = -2.5 + (x_pixel / (double)(X - 1)) * (1.0 - (-2.5));
			double cy = 1.0 - (y_pixel / (double)(Y - 1)) * (1.0 - (-1.0));
			int iteration = 0;
			while (x * x + y * y <= 2.0 * 2.0 && iteration < MAX_ITER) {
				double x_tmp = x * x - y * y + cx;
				y = 2.0 * x * y + cy;
				x = x_tmp;
				iteration++;
			}
			uint8_t norm_iteration = (uint8_t)((iteration * 255) / MAX_ITER);
			image[y_pixel][x_pixel] = norm_iteration;
		}
	}
}

static double monotonic_seconds(void) {
	struct timespec ts;
	if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
		perror("clock_gettime");
		exit(EXIT_FAILURE);
	}
	return (double)ts.tv_sec + 1e-9 * (double)ts.tv_nsec;
}


double benchmark_calc_mandelbrot(uint8_t image[Y][X]) {
	double t0 = monotonic_seconds();
	calc_mandelbrot(image);
	return monotonic_seconds() - t0;
}

int main(int argc, char **argv) {
	uint8_t image[Y][X];
	int do_benchmark = (argc >= 2 && strcmp(argv[1], "--benchmark") == 0);

	if (do_benchmark) {
		double sec = benchmark_calc_mandelbrot(image);

		printf("%.2f\n", sec);
	} else {
		calc_mandelbrot(image);
	}

	const int channel_nr = 1, stride_bytes = 0;
	stbi_write_png("mandelbrot.png", X, Y, channel_nr, image, stride_bytes);
	return EXIT_SUCCESS;
}