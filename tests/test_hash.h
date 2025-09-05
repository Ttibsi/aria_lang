#define ARIA_HASH_IMPLEMENTATION
#include "aria_hash.h"
#include "onetest.h"


static int test_mapCreate(void) {
    Map* map = mapCreate();

    for (int i = 0; i < MAPSIZE; i++) {
        onetest_assert(map->items[i] == NULL);
    }

    return 0;
}

static int test_hash(void) {
    uint32_t hash1 = hash("hello");
    uint32_t hash2 = hash("hello");
    onetest_assert(hash1 == hash2);

    uint32_t hash3 = hash("world");
    onetest_assert(hash1 != hash3);

    onetest_assert(hash("test") < MAPSIZE);
    onetest_assert(hash("another_test") < MAPSIZE);

    return 0;
}

static int test_mapInsert(void) {
    Map* map = mapCreate();

    // Insert a key-value pair
    mapInsert(map, "hello", 5);

    // Find the bucket where it should be stored
    int hash_index = hash("hello");
    Entry* bucket = map->items[hash_index];

    // The actual data should be in the next entry
    onetest_assert(bucket->next == NULL);
    onetest_assert(strcmp(bucket->key, "hello") == 0);
    onetest_assert(bucket->value == 5);

    // Test inserting multiple items in same bucket (collision handling)
    onetest_assert(hash("Hello") == hash("anything"));
    mapInsert(map, "Hello", 10);
    mapInsert(map, "anything", 20);

    mapFree(map);
    return 0;
}

static int test_mapRemove(void) {
    Map* map = mapCreate();
    mapInsert(map, "Hello", 5);
    mapInsert(map, "anything", 10);

    int loc = hash("Hello");
    onetest_assert(hash("Hello") == hash("anything"));

    onetest_assert(map->items[hash("Hello")]->key == "Hello");
    onetest_assert(map->items[hash("Hello")]->next->key == "anything");
    onetest_assert(mapFind(map, "Hello") == 5);

    mapRemove(map, "Hello");
    onetest_assert(map->items[hash("Hello")]->key == "anything");
    onetest_assert(mapFind(map, "Hello") == 10);

    return 0;
}

static int test_mapExists(void) {
    Map* map = mapCreate();
    mapInsert(map, "Hello", 5);
    mapInsert(map, "anything", 10);
    mapInsert(map, "foo", 2);

    onetest_assert(mapExists(map, "foo"));
    onetest_assert(!mapExists(map, "bar"));

    // Check for items that should be in the same bucket
    onetest_assert(mapExists(map, "Hello"));
    onetest_assert(mapExists(map, "anything"));

    return 0;
}

static int test_mapFind(void) {
    Map* map = mapCreate();

    mapInsert(map, "key1", 100);
    mapInsert(map, "key2", 200);
    mapInsert(map, "key3", 300);

    onetest_assert(mapFind(map, "key1") == 100);
    onetest_assert(mapFind(map, "key2") == 200);
    onetest_assert(mapFind(map, "key3") == 300);

    mapFree(map);
    return 0;
}
