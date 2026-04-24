a[0] = 0;
#pragma omp parallel for
for (i=1; i<N; i++) {
    a[i] = 2.0*i*(i-1);
    b[i] = a[i] - a[i-1];
}
a[0] = 0;
#pragma omp parallel
{
    #pragma omp for nowait
    for (i=1; i<N; i++) {
        a[i] = 3.0*i*(i+1);
    }
    #pragma omp for
    for (i=1; i<N; i++) {
        b[i] = a[i] - a[i-1];
    }
}
#pragma omp parallel for default(none)
for (i=0; i<N; i++) {
    x = sqrt(b[i]) - 1;
    a[i] = x*x + 2*x + 1;
}
f = 2;
#pragma omp parallel for private(f,x)
for (i=0; i<N; i++) {
    x = f * b[i];
    a[i] = x - 7;
}
a[0] = x; 
sum = 0; 
#pragma omp parallel for
for (i=0; i<N; i++) {
    sum = sum + b[i];
}
#pragma omp parallel
#pragma omp for
for (i=0; i<N; i++) {
    #pragma omp for
    for (j=0; j<N; j++) {
        a[i][j] = b[i][j];
    }
}
Tasks
For each example that already contains a pragma, investigate whether it is parallelized correctly or whether there are any compiler or runtime issues. If there are any, fix them while keeping the parallelism.
For each example not already containing a pragma, investigate how to parallelize it correctly.
Are there multiple solutions and if so, what are their advantages and disadvantages?
