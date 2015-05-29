import os
import statistics
import sys
import babeltrace

def pairwise(iterable):
    a = iter(iterable)
    return zip(a, a)

class Test():

	def __init__(self, threads, iters, lock_type):
		self.lock_times = {}
		self.test_times = {}
		self.thread_test = {}
		self.threads = threads
		self.iters = iters
		self.lock_type = lock_type

	def add_lock_time(self, tid, et):
		try:
			self.lock_times[tid].append(et)
		except KeyError:
			self.lock_times[tid] = [et]

	def start_test_thread(self, tid):
		self.thread_test[tid] = True

	def end_test_thread(self, tid):
		del self.thread_test[tid]

	def is_test_thread(self, tid):
		return self.thread_test.get(tid, False)

	def set_test_time(self, tid, et):
		self.test_times[tid] = et

	def calc_lock_stats(self):
		times = {k:statistics.mean(v) for k, v in self.lock_times.items()}
		mean = statistics.mean(times.values()) / 1000.0
		stddev = statistics.pstdev(times.values()) / 1000.0
		total = sum(times.values()) / 1000.0
		return mean, stddev, total

	def calc_test_stats(self):
		mean = statistics.mean(self.test_times.values()) / 1000.0
		stddev = statistics.pstdev(self.test_times.values()) / 1000.0
		total = sum(self.test_times.values()) / 1000.0
		return mean, stddev, total


def collect_tests(path):
	col = babeltrace.TraceCollection()

	#~ if col.add_trace('%s/kernel' % path, 'ctf') is None:
		#~ raise RuntimeError('Cannot add kernel trace')

	if col.add_trace('%s/ust/uid/1000/64-bit' % path, 'ctf') is None:
		raise RuntimeError('Cannot add user trace')

	tests = []
	test = None

	for event in col.events:

		if event.name == 'tl:start_test':
			test = Test(event['threads'], event['iters'], event['lock_type'])
			tests.append(test)

		elif event.name == 'tl:end_test':
			test = None

		elif test:
			if event.name == 'tl:start_test_thread':
				test.start_test_thread(event['vtid'])

			elif event.name == 'ust_pthread:pthread_mutex_lock_acq' or \
				 event.name == 'ust_pthread:pthread_rwlock_rdlock_acq' or \
				 event.name == 'ust_pthread:pthread_rwlock_wrlock_acq':
				if test.is_test_thread(event['vtid']):
					test.add_lock_time(event['vtid'], event['elapsed_time'])

			elif event.name == 'tl:end_test_thread':
				test.end_test_thread(event['vtid'])
				test.set_test_time(event['vtid'], event['elapsed_time'])

	return tests


#~ def show_stats(tests):
	#~ print('{:20s} {:14s} {:14s} {:15s} {:14s} {:14s} {:15s}'.format("", "mean(t)", "stddev(t)", "total(t)", "mean(l)", "stddev(l)", "total(l)"))
	#~ for test in tests:
		#~ test.lock_type, test.threads, test.iters,
		#~ mt, st, tt = test.calc_test_stats()
		#~ ml, sl, tl = test.calc_lock_stats()
		#~ print('{:6s} {:6d} {:6d} {:14.2f} {:14.2f} {:15.2f} {:14.2f} {:14.2f} {:15.2f}'.format(test.lock_type, test.threads, test.iters, mt, st, tt, ml, sl, tl,))

def show_stats(tests):
	print('{:8s}  {:30s}  {:30s}  {:30s}  {:30s} '.format("#threads", "lock mean (mutex,rwlock)", "lock total (mutex,rwlock)", "test mean (mutex,rwlock)", "test total (mutex,rwlock)"))
	for mutex, rwlock in pairwise(tests):
		mml, _, mtl = mutex.calc_lock_stats()
		mmt, _, mtt = mutex.calc_test_stats()
		rml, _, rtl = rwlock.calc_lock_stats()
		rmt, _, rtt = rwlock.calc_test_stats()
		print('{:8d}  {:14.2f} {:15.2f}  {:14.2f} {:15.2f}  {:14.2f} {:15.2f}  {:14.2f} {:15.2f}'.format(mutex.threads, mml, rml, mtl, rtl, mmt, rmt, mtt, rtt))


if __name__ == '__main__':
	lttng_path = '%s/lttng-traces' % os.path.expanduser("~")
	sessions = sorted('%s/%s' % (lttng_path, s)
		for s in os.listdir(lttng_path) if s.startswith('test_locks'))
	for session in sorted(sessions):
		tests = collect_tests(session)
		show_stats(tests)
