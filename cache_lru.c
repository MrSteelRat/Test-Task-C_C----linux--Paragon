#include "cache_lru.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Structure for a cache element
struct Node {
    int key;
    int value;
    struct Node *next;
    struct Node *prev;
};

// Structure for a hash table element
typedef struct hashmap {
    int key;
    struct Node *node;
} Hashmap;

// Structure for LRU cache
struct cache_lru {
    int capacity;
    int size;
    struct Node *head;
    struct Node *tail;
    Hashmap **map;
    int map_size;
};

// Hash function
int hash_one(int key, int M) {
    return key % M;
}

int hash_two(int key, int M) {
    return 1 + (key % (M - 1));
}

// Initializing a hash table
void init_table(Hashmap **hashmap, int M) {
    for (int i = 0; i < M; i++) {
        hashmap[i] = NULL;
    }
}

// Getting a value by key from a hash table
Hashmap *get(int key, Hashmap **hashmap, int M) {
    int hash1 = hash_one(key, M);
    int hash2 = hash_two(key, M);
    int i = 0;

    while (1) {
        Hashmap *currentPair = hashmap[(hash1 + i * hash2) % M];
        if (currentPair == NULL) {
            return NULL;
        }
        if (currentPair->key == key) {
            return currentPair;
        }
        i++;
    }
}

// Inserting into a hash table
void insert_hash(int key, Hashmap **hashmap, Hashmap *pair, int M) {
    int hash1 = hash_one(key, M);
    int index = hash1;

    if (hashmap[index] != NULL) {
        int index2 = hash_two(key, M);
        int i = 0;
        while (1) {
            int newIndex = (index + i * index2) % M;
            if (hashmap[newIndex] == NULL) {
                hashmap[newIndex] = pair;
                break;
            }
            i++;
        }
    } else {
        hashmap[index] = pair;
    }
}

// Removing from a hash table
void remove_hash(int key, Hashmap **hashmap, int M) {
    int hash1 = hash_one(key, M);
    int hash2 = hash_two(key, M);
    int i = 0;
    while (1) {
        Hashmap *currentPair = hashmap[(hash1 + i * hash2) % M];
        if (currentPair == NULL) {
            return;
        }
        if (currentPair->key == key) {
            hashmap[(hash1 + i * hash2) % M] = NULL;
        }
        i++;
    }
}

// Removing a node from the cache
void delete_node(struct cache_lru *cache, struct Node *rm) {
    if (rm == NULL) return;
    
    if (rm->prev != NULL) {
        rm->prev->next = rm->next;
    } else {
        cache->head = rm->next;
    }

    if (rm->next != NULL) {
        rm->next->prev = rm->prev;
    } else {
        cache->tail = rm->prev;
    }
}

// Adding a node to the beginning of the list
void put_on_top(struct cache_lru *cache, struct Node *newNode) {
    newNode->next = cache->head;
    newNode->prev = NULL;

    if (cache->head != NULL) {
        cache->head->prev = newNode;
    }

    cache->head = newNode;

    if (cache->tail == NULL) {
        cache->tail = newNode;
    }
}

// Function for creating a cache
struct cache_lru *cache_lru_create(int capacity) {
    struct cache_lru *cache = (struct cache_lru *)malloc(sizeof(struct cache_lru));
    cache->capacity = capacity;
    cache->size = 0;
    cache->head = NULL;
    cache->tail = NULL;
    
    cache->map_size = capacity * 2;
    cache->map = (Hashmap **)malloc(sizeof(Hashmap *) * cache->map_size);
    init_table(cache->map, cache->map_size);
    
    return cache;
}

// Destroying the cache
void cache_lru_destroy(struct cache_lru **cache) {
    free(*cache);
    *cache = NULL;
}

// Function for adding an element to the cache
void cache_lru_put(struct cache_lru *cache, int key, int value) {
    Hashmap *currentHash = get(key, cache->map, cache->map_size);

    if (currentHash != NULL) {
        currentHash->node->value = value;
        delete_node(cache, currentHash->node);
        put_on_top(cache, currentHash->node);
    } else {
        if (cache->size >= cache->capacity) {
            remove_hash(cache->tail->key, cache->map, cache->map_size);
            delete_node(cache, cache->tail);
            cache->size--;
        }
        struct Node *newNode = (struct Node *)malloc(sizeof(struct Node));
        newNode->key = key;
        newNode->value = value;
        newNode->next = NULL;
        newNode->prev = NULL;
        
        Hashmap *hash = (Hashmap *)malloc(sizeof(Hashmap));
        hash->key = key;
        hash->node = newNode;
        
        insert_hash(key, cache->map, hash, cache->map_size);
        put_on_top(cache, newNode);
        cache->size++;
    }
}

// Function for getting an element from the cache
int cache_lru_get(struct cache_lru *cache, int key) {
    Hashmap *currentHash = get(key, cache->map, cache->map_size);
    
    if (currentHash != NULL) {
        delete_node(cache, currentHash->node);
        put_on_top(cache, currentHash->node);
        return currentHash->node->value;
    } else {
        return -1;
    }
}
