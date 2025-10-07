#define ARIA_BUFFER_IMPL
#include "aria_buffer.h"

#include "onetest.h"

static int test_bufferCreate(void) {
    Aria_Buffer* buf = bufferCreate(sizeof(int), 8);
    onetest_assert(buf->elem_size == 4);
    onetest_assert(buf->capacity == 8);
    onetest_assert(buf->size == 0);

    bufferFree(buf);
    return 0;
}

static int test_bufferInsert(void) {
    Aria_Buffer* buf = bufferCreate(sizeof(int), 8);
    int* num = (int*)malloc(sizeof(int));
    *num = 5;
    bufferInsert(buf, num);
    onetest_assert(*(int*)buf->items == 5);

    bufferFree(buf);
    return 0;
}

static int test_bufferGet(void) {
    Aria_Buffer* buf = bufferCreate(sizeof(int), 8);
    int* num = (int*)malloc(sizeof(int));
    *num = 5;
    bufferInsert(buf, num);

    int actual = 5;
    onetest_assert(*(int*)bufferGet(buf, 0) == actual);

    bufferFree(buf);
    return 0;
}

static int test_bufferPeek(void) {
    Aria_Buffer* buf = bufferCreate(sizeof(int), 8);
    int* num = (int*)malloc(sizeof(int));
    *num = 1;
    bufferInsert(buf, num);

    *num = 2;
    bufferInsert(buf, num);

    *num = 3;
    bufferInsert(buf, num);

    onetest_assert(*(int*)bufferPeek(buf) == 3);
    return 0;
}

static int test_bufferCopy(void) {
    Aria_Buffer *src = bufferCreate(sizeof(int), 8);
    int vals[] = {10, 20, 30};
    for (size_t i = 0; i < sizeof(vals)/sizeof(vals[0]); ++i) {
        int *p = (int *)malloc(sizeof(int));
        *p = vals[i];
        bufferInsert(src, p);
    }

    Aria_Buffer *dest = bufferCreate(sizeof(int), 4);   /* capacity will be overwritten */
    bufferCopy(dest, src);

    onetest_assert(dest->elem_size == src->elem_size);
    onetest_assert(dest->capacity  == src->capacity);
    onetest_assert(dest->size      == src->size);

    for (size_t i = 0; i < src->size; ++i) {
        int src_val = *(int *)bufferGet(src, i);
        int dest_val = *(int *)bufferGet(dest, i);
        onetest_assert(src_val == dest_val);
    }

    bufferFree(src);
    bufferFree(dest);
    return 0;
}
