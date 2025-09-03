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
    Entry* items[MAPSIZE];
} Map;

static Map* mapCreate();
static void mapInsert(Map* map, char* key, int value);
static void mapRemove(Map* map, char* key);
static int mapFind(Map* map, const char* key);

#ifdef ARIA_HASH_IMPLEMENTATION
static Map* mapCreate() {
    Map* map = (Map*)malloc(sizeof(Map) * size);
    for (size_t i = 0; i < MAPSIZE; i++) { map->items[i] = (Entry*)malloc(sizeof(Entry)); }
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

static void mapInsert(Map* map, char* key, int value) {
    int hash_num = hash(key);
    Entry* current = map->items[hash_num];
    while (current->next != NULL) { current = current->next; }

    current->next = (Entry*)malloc(sizeof(Entry));
    current->next->prev = current;
    current->next->next = NULL;
    current->key = key;
    current->value = value;
}

static void mapRemove(Map* map, char* key) {
    int hash_num = hash(key);
    Entry* current = map->items[hash_num];
    while (strcmp(current->key, key) != 0) { current = current->next; }

    current->next->prev = current->prev;
    current->prev->next = current->next;
    free(current);
}

static int mapFind(Map* map, const char* key) {
    int hash_num = hash(key);
    Entry* current = map->items[hash_num];
    while (strcmp(current->key, key) != 0) { current = current->next; }
    return current->value;
}

static void mapFree(Map* map) {
    for (int i = 0; i < MAPSIZE; i++) {
        Entry* current = map->items[i];
        while (current->next != NULL) { current = current->next; }

        if (current->prev != NULL) {
            current = current->prev;
        }

        while (current->prev != NULL) {
            free(current->next);
            current = current->prev;
        }
    }
    free(map);
}

#endif // ARIA_HASH_IMPLEMENTATION
#endif // ARIA_HASH_H
