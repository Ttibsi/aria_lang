#define ARIA_HASH_IMPL
#include "aria_hash.h"
#include "onetest.h"


static int test_mapCreate(void) {
    Map* map = mapCreate(8);

    for (int i = 0; i < MAPSIZE; i++) {
        onetest_assert(map->items[i] == NULL);
    }

    mapFree(map);
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
    Map* map = mapCreate(sizeof(int));

    // Insert a key-value pair
    int val = 5;
    mapInsert(map, "hello", &val);

    // Find the bucket where it should be stored
    int hash_index = hash("hello");
    Entry* bucket = map->items[hash_index];

    // The actual data should be in the next entry
    onetest_assert(bucket->next == NULL);
    onetest_assert(strcmp(bucket->key, "hello") == 0);
    onetest_assert(*(int*)bucket->value == val);

    // Test inserting multiple items in same bucket (collision handling)
    int val1 = 10;
    int val2 = 20;
    onetest_assert(hash("Hello") == hash("anything"));

    int ret = mapInsert(map, "Hello", &val1);
    onetest_assert(ret == 1);

    ret = mapInsert(map, "anything", &val2);
    onetest_assert(ret == 0);

    mapFree(map);
    return 0;
}

static int test_mapRemove(void) {
    Map* map = mapCreate(sizeof(int));
    int val1 = 10;
    int val2 = 20;
    mapInsert(map, "Hello", &val1);
    mapInsert(map, "anything", &val2);

    onetest_assert(hash("Hello") == hash("anything"));

    onetest_assert(strcmp(map->items[hash("Hello")]->key, "Hello") == 0);
    onetest_assert(strcmp(map->items[hash("Hello")]->next->key, "anything") == 0);
    onetest_assert(*(int*)mapFind(map, "Hello") == val1);

    mapRemove(map, "Hello");
    onetest_assert(strcmp(map->items[hash("Hello")]->key, "anything") == 0);
    onetest_assert(*(int*)mapFind(map, "Hello") == val2);

    mapFree(map);
    return 0;
}

static int test_mapExists(void) {
    Map* map = mapCreate(sizeof(int));
    int val1 = 10;
    int val2 = 20;
    mapInsert(map, "Hello", &val1);
    mapInsert(map, "anything", &val2);
    mapInsert(map, "foo", &val2);

    onetest_assert(mapExists(map, "foo"));
    onetest_assert(!mapExists(map, "bar"));

    // Check for items that should be in the same bucket
    onetest_assert(mapExists(map, "Hello"));
    onetest_assert(mapExists(map, "anything"));

    mapFree(map);
    return 0;
}

static int test_mapFind(void) {
    Map* map = mapCreate(sizeof(int));

    int val = 100;
    mapInsert(map, "key1", &val);
    onetest_assert(*(int*)mapFind(map, "key1") == val);

    val = 200;
    mapInsert(map, "key2", &val);
    onetest_assert(*(int*)mapFind(map, "key2") == val);

    val = 300;
    mapInsert(map, "key3", &val);
    onetest_assert(*(int*)mapFind(map, "key3") == val);

    mapFree(map);
    return 0;
}
