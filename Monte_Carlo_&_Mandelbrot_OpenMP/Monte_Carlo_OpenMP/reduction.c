#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

int main(int argc, char *argv[])
{
    long n = 700000000;
    long i, count = 0;
    double pi;
    double startTime, endTime;

    int num_threads = (argc >= 2) ? atoi(argv[1]) : 1;
    omp_set_num_threads(num_threads);

    startTime = omp_get_wtime();

#pragma omp parallel for reduction(+:count)
    for (i = 0; i < n; i++)
    {
        unsigned int seed = (unsigned int)(omp_get_thread_num() * 1000 + i);
        double x = (double)rand_r(&seed) / RAND_MAX;
        double y = (double)rand_r(&seed) / RAND_MAX;

        if (x * x + y * y <= 1)
        {
            count++;
        }
    }

    endTime = omp_get_wtime();

    pi = 4.0 * count / n;
    printf("Approximate value of pi: %f\n", pi);
    printf("time: %2.4f seconds\n", endTime - startTime);

    return 0;
}
