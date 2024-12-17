#include "cache_lru.h"
#include <stdlib.h>

struct cache_lru;

struct cache_lru *cache_lru_create(int capacity)
{
	return NULL;
}

void cache_lru_destroy(Cache_lru **cache)
{
}

void cache_lru_put(Cache_lru *cache, int key, int value)
{
}

int cache_lru_get(Cache_lru *cache, int key)
{
	return -1;
}
