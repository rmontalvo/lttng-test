#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "cache/cache.h"
#include "data.h"
#include "test_locks_tp/test_locks_tp.h"

#define ELAPSED_TIME(s, e)	(((e).tv_sec - (s).tv_sec) * 1e9 + ((e).tv_nsec - (s).tv_nsec))

struct {
	int num_iters;
	Cache* cache;
	void (*fn)(int);
	pthread_barrier_t barrier;
} shared;


void *run_thread(void *tid) {
	int i, id = (int)(long) tid;
	struct timespec start, end;
	int iters = shared.num_iters;

	pthread_barrier_wait(&shared.barrier);

	tracepoint(tl, start_test_thread, id);
	clock_gettime(CLOCK_MONOTONIC, &start);
	for (i = 0; i < iters; i++) {
		int r = rand() % DATA_SIZE_FULL;
		if ((rand() % 100) < 80) {
			cache_get(shared.cache, GET_KEY(r));
		} else {
			cache_put(shared.cache, GET_KEY(r), GET_VAL(r));
		}
	}
	clock_gettime(CLOCK_MONOTONIC, &end);
	tracepoint(tl, end_test_thread, id, ELAPSED_TIME(start, end));

	//~ printf("%d\n", id);
	pthread_exit(NULL);
}

void run_test(int num_threads, int num_iters, int cache_size, int lock_type) {
	int rc;
	long t;
	struct timespec start, end;
	pthread_t* threads = malloc(sizeof(pthread_t) * num_threads);

	shared.num_iters = num_iters;
	shared.cache = malloc(sizeof(Cache));
	cache_init(shared.cache, cache_size, lock_type);
	pthread_barrier_init(&shared.barrier, NULL, num_threads);

	tracepoint(tl, start_test, num_threads, num_iters, (!lock_type ? "mutex" : "rwlock"));
	clock_gettime(CLOCK_MONOTONIC, &start);
	for(t = 0; t < num_threads; t++) {
		rc = pthread_create(&threads[t], NULL, run_thread, (void *)t);
		if (rc != 0) {
			fprintf(stderr, "Error at pthread_create() with id %d\n", rc);
			exit(-1);
		}
	}
	for(t = 0; t < num_threads; t++) {
		pthread_join(threads[t], NULL);
	}
	clock_gettime(CLOCK_MONOTONIC, &end);
	tracepoint(tl, end_test, ELAPSED_TIME(start, end));

	cache_destroy(shared.cache);
	free(shared.cache);
	pthread_barrier_destroy(&shared.barrier);
	free(threads);
}

void run_tests() {
	int threads, iters, cache_size;

	srand(time(NULL));
	cache_size = DATA_SIZE_STAB;

	threads = 2;
	iters = 100;
	run_test(threads, iters, cache_size, MUTEX);
	run_test(threads, iters, cache_size, RWLOCK);

	threads = 4;
	iters = 100;
	run_test(threads, iters, cache_size, MUTEX);
	run_test(threads, iters, cache_size, RWLOCK);

	threads = 8;
	iters = 100;
	run_test(threads, iters, cache_size, MUTEX);
	run_test(threads, iters, cache_size, RWLOCK);

	threads = 16;
	iters = 100;
	run_test(threads, iters, cache_size, MUTEX);
	run_test(threads, iters, cache_size, RWLOCK);

	threads = 32;
	iters = 100;
	run_test(threads, iters, cache_size, MUTEX);
	run_test(threads, iters, cache_size, RWLOCK);

	threads = 64;
	iters = 100;
	run_test(threads, iters, cache_size, MUTEX);
	run_test(threads, iters, cache_size, RWLOCK);
}

int main (int argc, char *argv[]) {
	run_tests();
	return 0;
}
