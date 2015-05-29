#include "cache.h"

static void cache_wrlock(Cache *cache) {
	if (cache->lock_type == MUTEX) {
		if (pthread_mutex_lock(&cache->mutex) != 0) {
			printf("Error at pthread_mutex_lock() ...\n");
		}
	} else {
		if (pthread_rwlock_wrlock(&cache->rwlock) != 0) {
			printf("Error at pthread_rwlock_wrlock() ...\n");
		}
	}
}

static void cache_rdlock(Cache *cache) {
	if (cache->lock_type == MUTEX) {
		if (pthread_mutex_lock(&cache->mutex) != 0) {
			printf("Error at pthread_mutex_lock() ...\n");
		}
	} else {
		if (pthread_rwlock_rdlock(&cache->rwlock) != 0) {
			printf("Error at pthread_rwlock_rdlock() ...\n");
		}
	}
}

static void cache_unlock(Cache *cache) {
	if (cache->lock_type == MUTEX) {
		if (pthread_mutex_unlock(&cache->mutex) != 0) {
			printf("Error at pthread_mutex_unlock() ...\n");
		}
	} else {
		if (pthread_rwlock_unlock(&cache->rwlock) != 0) {
			printf("Error at pthread_rwlock_unlock() ...\n");
		}
	}
}

void cache_init(Cache *cache, int size, int lock_type) {
	cache->size = size;
	cache->head = NULL;
	cache->lock_type = lock_type;
	if (lock_type == MUTEX)
		pthread_mutex_init(&cache->mutex, NULL);
	else
		pthread_rwlock_init(&cache->rwlock, NULL);
}

char* cache_get(Cache *cache, char *key) {
	struct CacheEntry* entry;
	char *val = NULL;

	//~ lock
	cache_rdlock(cache);
	HASH_FIND_STR(cache->head, key, entry);
	if (entry) {
		clock_gettime(CLOCK_MONOTONIC, &entry->ts);
		val = entry->val;
	}
	//~ unlock
	cache_unlock(cache);

	return val;
}

void cache_put(Cache *cache, char *key, char *val) {
	struct CacheEntry *entry, *cur, *tmp, *old = NULL;

	entry = malloc(sizeof(struct CacheEntry));
	entry->key = strdup(key);
	entry->val = strdup(val);

	//~ lock
	cache_wrlock(cache);
	HASH_FIND_STR(cache->head, key, tmp);
	if (tmp) {
		HASH_DEL(cache->head, tmp);

	} else if (HASH_COUNT(cache->head)  >= cache->size) {
		HASH_ITER(hh, cache->head, cur, tmp) {
			if (old != NULL) {
				if (cur->ts.tv_sec <= old->ts.tv_sec &&
						cur->ts.tv_nsec < old->ts.tv_nsec)
					old = cur;
			} else {
				old = cur;
			}
		}
		HASH_DEL(cache->head, old);
	}
	clock_gettime(CLOCK_MONOTONIC, &entry->ts);
	HASH_ADD_KEYPTR(hh, cache->head, entry->key, strlen(entry->key), entry);
	//~ unlock
	cache_unlock(cache);

	if (old) free(old);
}

void cache_iterate(Cache* cache, void (*fn)(char *, char *)) {
	struct CacheEntry *entry, *tmp;

	//~ lock
	HASH_ITER(hh, cache->head, entry, tmp) {
		fn(strdup(entry->key), strdup(entry->val));
	}
	//~ unlock
}

void cache_destroy(Cache *cache) {
	if (cache->lock_type == MUTEX) {
		pthread_mutex_destroy(&cache->mutex);
	} else {
		pthread_rwlock_destroy(&cache->rwlock);
	}
}
