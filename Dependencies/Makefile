CC      = gcc
CFLAGS  = -O2 -fopenmp
LDFLAGS = -lm

all: exercise2

exercise2: exercise2.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f exercise2

run: exercise2
	OMP_NUM_THREADS=1 ./exercise2
	OMP_NUM_THREADS=2 ./exercise2
	OMP_NUM_THREADS=4 ./exercise2
	OMP_NUM_THREADS=8 ./exercise2
