#ifndef _CACHE_H_
#define _CACHE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "uthash.h"

#define MUTEX	0
#define RWLOCK	1

struct CacheEntry {
	char* key;
	char* val;
	UT_hash_handle hh;
	struct timespec ts;
};

typedef struct {
	int size;
	struct CacheEntry* head;
	int lock_type;
	pthread_mutex_t mutex;
	pthread_rwlock_t rwlock;
} Cache;

void cache_init(Cache *cache, int size, int lock_type);
char* cache_get(Cache* cache, char* key);
void cache_put(Cache* cache, char *key, char *val);
void cache_iterate(Cache* cache, void (*fn)(char *, char *));
void cache_destroy(Cache *cache);

#endif /* _CACHE_H_ */
