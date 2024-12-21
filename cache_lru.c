#include "cache_lru.h"
#include <stdlib.h>
#include <stdio.h>

// Node structure for doubly linked list
typedef struct node {
    int key;
    int value;
    struct node* prev;
    struct node* next;
} Node;

// LRU cache structure
struct cache_lru {
    int capacity;
    int size;
    Node *head;
    Node *tail;
    // Hash table for quick look-up (chaining for collisions)
    Node **hash_table;
};

static int hash(int key, int capacity) {
    return key % capacity;
}

static void move_to_front(Cache_lru *cache, Node *node) {
    if (cache->head == node) return;

    // Detach the node
    if (node->prev) node->prev->next = node->next;
    if (node->next) node->next->prev = node->prev;

    if (cache->tail == node) {
        cache->tail = node->prev;
    }

    // Move the node to the front
    node->next = cache->head;
    node->prev = NULL;
    if (cache->head) cache->head->prev = node;
    cache->head = node;

    if (!cache->tail) {
        cache->tail = node;
    }
}

// Function to remove a node from the list
static void remove_node(Cache_lru *cache, Node *node) {
    if (node->prev) node->prev->next = node->next;
    if (node->next) node->next->prev = node->prev;

    if (cache->head == node) cache->head = node->next;
    if (cache->tail == node) cache->tail = node->prev;

    free(node);
}

// Eviction function: Remove the least recently used node (from the tail)
static void evict(Cache_lru *cache) {
    if (cache->tail) {
        Node *node = cache->tail;
        remove_node(cache, node);

        // Remove the node from the hash table
        int index = hash(node->key, cache->capacity);
        Node *prev_node = cache->hash_table[index];
        if (prev_node == node) {
            cache->hash_table[index] = NULL;
        } else {
            // Handle collision: find the node and remove it from the chain
            while (prev_node && prev_node->next != node) {
                prev_node = prev_node->next;
            }
            if (prev_node) {
                prev_node->next = node->next;
            }
        }

        cache->size--;
    }
}

Cache_lru *cache_lru_create(int capacity) {
    if (capacity <= 0) return NULL;

    Cache_lru *cache = (Cache_lru *)malloc(sizeof(Cache_lru));
    if (!cache) return NULL;

    cache->capacity = capacity;
    cache->size = 0;
    cache->head = NULL;
    cache->tail = NULL;

    cache->hash_table = (Node **)calloc(capacity, sizeof(Node *));
    if (!cache->hash_table) {
        free(cache);
        return NULL;
    }

    return cache;
}

void cache_lru_destroy(Cache_lru **cache) {
    if (!cache || !(*cache)) return;

    Node *current = (*cache)->head;
    while (current) {
        Node *next = current->next;
        free(current);
        current = next;
    }

    free((*cache)->hash_table);
    free(*cache);
    *cache = NULL;
}

void cache_lru_put(Cache_lru *cache, int key, int value) {
    if (!cache) return;

    int index = hash(key, cache->capacity);
    Node *node = cache->hash_table[index];

    // Search for the node in the hash table chain
    while (node && node->key != key) {
        node = node->next;
    }

    if (node) {
        // If the key exists, update the value and move it to the front
        node->value = value;
        move_to_front(cache, node);
    } else {
        // If the key doesn't exist, create a new node
        if (cache->size == cache->capacity) {
            evict(cache); // Remove the least recently used element
        }

        node = (Node *)malloc(sizeof(Node));
        node->key = key;
        node->value = value;
        node->prev = NULL;
        node->next = cache->head;

        if (cache->head) cache->head->prev = node;
        cache->head = node;

        if (!cache->tail) {
            cache->tail = node;
        }

        // Insert the node into the hash table chain
        node->next = cache->hash_table[index];
        cache->hash_table[index] = node;
        cache->size++;
    }
}

int cache_lru_get(Cache_lru *cache, int key) {
    if (!cache) return -1;

    int index = hash(key, cache->capacity);
    Node *node = cache->hash_table[index];

    // Search for the node in the hash table chain
    while (node && node->key != key) {
        node = node->next;
    }

    if (!node) return -1;

    // Move the found node to the front (mark it as recently used)
    move_to_front(cache, node);
    return node->value;
}
