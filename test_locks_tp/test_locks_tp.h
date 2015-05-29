
#undef TRACEPOINT_PROVIDER
#define TRACEPOINT_PROVIDER tl

#undef TRACEPOINT_INCLUDE
#define TRACEPOINT_INCLUDE "./test_locks_tp.h"

#if !defined(TEST_LOCKS_TP_H) || defined(TRACEPOINT_HEADER_MULTI_READ)
#define TEST_LOCKS_TP_H

#include <lttng/tracepoint.h>

TRACEPOINT_EVENT(
	tl,
	start_test,
	TP_ARGS(
		int, threads,
		int, iters,
		char*, lock
	),
	TP_FIELDS(
		ctf_integer(int, threads, threads)
		ctf_integer(int, iters, iters)
		ctf_string(lock_type, lock)
	)
)

TRACEPOINT_EVENT(
	tl,
	end_test,
	TP_ARGS(
		unsigned long, elapsed_time
	),
	TP_FIELDS(
		ctf_integer(unsigned long, elapsed_time, elapsed_time)
	)
)

TRACEPOINT_EVENT(
	tl,
	start_test_thread,
	TP_ARGS(
		int, tid
	),
	TP_FIELDS(
		ctf_integer(int, tid, tid)
	)
)

TRACEPOINT_EVENT(
	tl,
	end_test_thread,
	TP_ARGS(
		int, tid,
		unsigned long, elapsed_time
	),
	TP_FIELDS(
		ctf_integer(int, tid, tid)
		ctf_integer(unsigned long, elapsed_time, elapsed_time)
	)
)

#endif /* TEST_LOCKS_TP_H */

#include <lttng/tracepoint-event.h>
