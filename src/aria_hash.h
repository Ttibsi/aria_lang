#ifndef ARIA_HASH_H
#define ARIA_HASH_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
    Map* map = (Map*)malloc(sizeof(Map));
    for (size_t i = 0; i < MAPSIZE; i++) { map->items[i] = NULL; }
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

    if (current == NULL) {
        current = malloc(sizeof(Entry));
        current->key = key;
        current->value = value;
        current->next = NULL;
        current->prev = NULL;

        map->items[hash_num] = current;

        return;
    }

    while (current->next != NULL) { current = current->next; }

    current->next = malloc(sizeof(Entry));
    current->next->key = key;
    current->next->value = value;
    current->next->next = NULL;
    current->next->prev = current;
}

static void mapRemove(Map* map, char* key) {
    int hash_num = hash(key);
    Entry* current = map->items[hash_num];
    if (current == NULL) { return; }

    while (strcmp(current->key, key) != 0) {
        current = current->next;
    }

    if (current->next != NULL && current->prev != NULL) {
        current->next->prev = current->prev;
        current->prev->next = current->next;
    } else if (current->next != NULL && current->prev == NULL) {
        // first entry
        map->items[hash_num] = current->next;
        current->next->prev = NULL;
    } else if (current->next == NULL && current->prev != NULL) {
        current->prev->next = NULL;
    }

    free(current);
}

static bool mapExists(Map* map, const char* key) {
    int hash_num = hash(key);
    Entry* current = map->items[hash_num];

    if (current == NULL) {
        return false;
    }

    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            return true;
        }
        current = current->next;
    }

    return false;
}

// This assumes the value exists already
static int mapFind(Map* map, const char* key) {
    int hash_num = hash(key);
    Entry* current = map->items[hash_num];
    if (current == NULL) { return 0; }

    while (strcmp(current->key, key) != 0 && current->next != NULL) {
        current = current->next;
    }

    if (current == NULL) { return 0; }
    return current->value;
}

static void mapFree(Map* map) {
    if (map == NULL) { return; }

    for (int i = 0; i < MAPSIZE; i++) {
        Entry* current = map->items[i];

        while (current != NULL) {
            Entry* temp = current;
            current = current->next;
            free(temp);
        }
    }

    free(map);
}

#endif // ARIA_HASH_IMPLEMENTATION
#endif // ARIA_HASH_H
