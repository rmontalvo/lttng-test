CC = gcc
CFLAGS = -Wall
LDFLAGS = ./pthread_wrapper/liblttng-ust-pthread-wrapper.so
LIBFLAGS = -shared -fPIC -ldl -llttng-ust
LTTNGFLAGS = -llttng-ust -ldl
PTHREADFLAGS =  -pthread

all: test_locks

stats:
	python3 stats.py

test: test_locks pthread_wrapper/liblttng-ust-pthread-wrapper.so
	lttng create test_locks
	lttng enable-event -u ust_pthread:pthread_mutex_lock_req,ust_pthread:pthread_mutex_lock_acq
	lttng enable-event -u ust_pthread:pthread_rwlock_rdlock_acq,ust_pthread:pthread_rwlock_wrlock_acq
	lttng enable-event -u tl:start_test,tl:end_test,tl:start_test_thread,tl:end_test_thread
	lttng add-context -u -c channel0 -t procname -t vpid -t vtid
	lttng start
	LD_PRELOAD=$(LDFLAGS) ./test_locks
	lttng stop
	lttng view > test_locks_trace
	lttng destroy

test_locks: test_locks.c cache/cache.o test_locks_tp/test_locks_tp.o
	$(CC) -o test_locks test_locks.c cache/cache.o test_locks_tp/test_locks_tp.o $(CFLAGS) $(PTHREADFLAGS) $(LTTNGFLAGS)

cache: cache/cache.o
cache/cache.o: cache/cache.c cache/cache.h
	cd cache; $(CC) -c cache.c $(CFLAGS) $(PTHREADFLAGS)

tp: test_locks_tp/test_locks_tp.o
test_locks_tp/test_locks_tp.o: test_locks_tp/test_locks_tp.tp
	cd test_locks_tp; lttng-gen-tp test_locks_tp.tp

wrapper: pthread_wrapper/liblttng-ust-pthread-wrapper.so
pthread_wrapper/liblttng-ust-pthread-wrapper.so: pthread_wrapper/lttng-ust-pthread.c pthread_wrapper/ust_pthread.h
	cd pthread_wrapper; C_INCLUDE_PATH=. $(CC) -o liblttng-ust-pthread-wrapper.so lttng-ust-pthread.c $(CFLAGS) $(LIBFLAGS)

clean:
	rm -rf test_locks
	rm -rf cache/cache.o
	rm -rf pthread_wrapper/liblttng-ust-pthread-wrapper.so
	cd test_locks_tp; rm -rf test_locks_tp.o test_locks_tp.c test_locks_tp.h
