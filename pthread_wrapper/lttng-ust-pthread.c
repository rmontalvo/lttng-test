/*
 * Copyright (C) 2013  Mentor Graphics
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#define _GNU_SOURCE
#include <lttng/ust-dlfcn.h>
#include <time.h>
#include <pthread.h>

#define TRACEPOINT_DEFINE
#define TRACEPOINT_CREATE_PROBES
#include "ust_pthread.h"

#define ELAPSED_TIME(s, e)	(((e).tv_sec - (s).tv_sec) * 1e9 + ((e).tv_nsec - (s).tv_nsec))

static __thread int thread_in_trace;

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
	static int (*mutex_lock)(pthread_mutex_t *);
	struct timespec start, end;
	int retval;

	if (!mutex_lock) {
		mutex_lock = dlsym(RTLD_NEXT, "pthread_mutex_lock");
		if (!mutex_lock) {
			if (thread_in_trace) {
				abort();
			}
			fprintf(stderr, "unable to initialize pthread wrapper library.\n");
			return EINVAL;
		}
	}
	if (thread_in_trace) {
		return mutex_lock(mutex);
	}

	thread_in_trace = 1;
	tracepoint(ust_pthread, pthread_mutex_lock_req, mutex);
	clock_gettime(CLOCK_MONOTONIC, &start);
	retval = mutex_lock(mutex);
	clock_gettime(CLOCK_MONOTONIC, &end);
	unsigned long int elapsed_time = ELAPSED_TIME(start, end);
	tracepoint(ust_pthread, pthread_mutex_lock_acq, mutex, retval, elapsed_time);
	thread_in_trace = 0;
	return retval;
}

int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
	static int (*mutex_trylock)(pthread_mutex_t *);
	int retval;

	if (!mutex_trylock) {
		mutex_trylock = dlsym(RTLD_NEXT, "pthread_mutex_trylock");
		if (!mutex_trylock) {
			if (thread_in_trace) {
				abort();
			}
			fprintf(stderr, "unable to initialize pthread wrapper library.\n");
			return EINVAL;
		}
	}
	if (thread_in_trace) {
		return mutex_trylock(mutex);
	}

	thread_in_trace = 1;
	retval = mutex_trylock(mutex);
	tracepoint(ust_pthread, pthread_mutex_trylock, mutex, retval);
	thread_in_trace = 0;
	return retval;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	static int (*mutex_unlock)(pthread_mutex_t *);
	int retval;

	if (!mutex_unlock) {
		mutex_unlock = dlsym(RTLD_NEXT, "pthread_mutex_unlock");
		if (!mutex_unlock) {
			if (thread_in_trace) {
				abort();
			}
			fprintf(stderr, "unable to initialize pthread wrapper library.\n");
			return EINVAL;
		}
	}
	if (thread_in_trace) {
		return mutex_unlock(mutex);
	}

	thread_in_trace = 1;
	retval = mutex_unlock(mutex);
	tracepoint(ust_pthread, pthread_mutex_unlock, mutex, retval);
	thread_in_trace = 0;
	return retval;
}

int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock)
{
	static int (*rwlock_rdlock)(pthread_rwlock_t *);
	struct timespec start, end;
	int retval;

	if (!rwlock_rdlock) {
		rwlock_rdlock = dlsym(RTLD_NEXT, "pthread_rwlock_rdlock");
		if (!rwlock_rdlock) {
			if (thread_in_trace) {
				abort();
			}
			fprintf(stderr, "unable to initialize pthread wrapper library.\n");
			return EINVAL;
		}
	}
	if (thread_in_trace) {
		return rwlock_rdlock(rwlock);
	}

	thread_in_trace = 1;
	tracepoint(ust_pthread, pthread_rwlock_rdlock_req, rwlock);
	clock_gettime(CLOCK_MONOTONIC, &start);
	retval = rwlock_rdlock(rwlock);
	clock_gettime(CLOCK_MONOTONIC, &end);
	unsigned long int elapsed_time = ELAPSED_TIME(start, end);
	tracepoint(ust_pthread, pthread_rwlock_rdlock_acq, rwlock, retval, elapsed_time);
	thread_in_trace = 0;
	return retval;
}

int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock)
{
	static int (*rwlock_wrlock)(pthread_rwlock_t *);
	struct timespec start, end;
	int retval;

	if (!rwlock_wrlock) {
		rwlock_wrlock = dlsym(RTLD_NEXT, "pthread_rwlock_wrlock");
		if (!rwlock_wrlock) {
			if (thread_in_trace) {
				abort();
			}
			fprintf(stderr, "unable to initialize pthread wrapper library.\n");
			return EINVAL;
		}
	}
	if (thread_in_trace) {
		return rwlock_wrlock(rwlock);
	}

	thread_in_trace = 1;
	tracepoint(ust_pthread, pthread_rwlock_wrlock_req, rwlock);
	clock_gettime(CLOCK_MONOTONIC, &start);
	retval = rwlock_wrlock(rwlock);
	clock_gettime(CLOCK_MONOTONIC, &end);
	unsigned long int elapsed_time = ELAPSED_TIME(start, end);
	tracepoint(ust_pthread, pthread_rwlock_wrlock_acq, rwlock, retval, elapsed_time);
	thread_in_trace = 0;
	return retval;
}
