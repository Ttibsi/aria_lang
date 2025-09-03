#ifndef ARIA_HASH_H
#define ARIA_HASH_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MAPSIZE 10

typedef struct _hash_entry {
    struct _hash_entry* next;
    struct _hash_entry* prev;
    char* key;
    int value;
} Entry;

typedef struct {
    Entry items[MAPSIZE];
} Map;

static Map* mapCreate(const int size);
static void mapInsert(Map* map, const Entry* entry);
static void mapRemove(Map* map, const Entry* entry);
static int mapFind(Map* map, const char* key);

#ifdef ARIA_HASH_IMPLEMENTATION
static Map* mapCreate(const int size) {
    Map* map = (Map*)malloc(sizeof(Map) * size);
    for (size_t i = 0; i < MAPSIZE; i++) { map->items[i] = (Entry){NULL, NULL, NULL, 0}; }
    return map;
}

// FNV-1a standard hashing formula sourced from craftinginterpreters
static uint32_t hash(const char* key) {
    uint32_t hash = 2166136261u;
    for (int i = 0; i < strlen(key); i++) {
        hash ^= (uint8_t)key[i];
        hash *= 16777619;
    }

    return hash % MAPSIZE;
}

static void mapInsert(Map* map, const Entry* entry) {
}

static void mapRemove(Map* map, const Entry* entry) {
}

static int mapFind(Map* map, const char* key) {
}

static void mapFree(Map* map) {
    free(map);
}

#endif // ARIA_HASH_IMPLEMENTATION
#endif // ARIA_HASH_H
