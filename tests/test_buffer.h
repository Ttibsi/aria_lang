#define ARIA_BUFFER_IMPL
#include "aria_buffer.h"

#include "onetest.h"

static int test_bufferCreate(void) {
    Aria_Buffer buf = bufferCreate(sizeof(int), 8);
    onetest_assert(buf.elem_size == 4);
    onetest_assert(buf.capacity == 8);
    onetest_assert(buf.size == 0);

    bufferFree(buf);
    return 0;
}

static int test_bufferInsert(void) {
    Aria_Buffer buf = bufferCreate(sizeof(int), 8);
    int* num = (int*)malloc(sizeof(int));
    *num = 5;
    bufferInsert(&buf, num);
    onetest_assert(*(int*)buf.items == 5);

    bufferFree(buf);
    return 0;
}

static int test_bufferGet(void) {
    Aria_Buffer buf = bufferCreate(sizeof(int), 8);
    int* num = (int*)malloc(sizeof(int));
    *num = 5;
    bufferInsert(&buf, num);

    int actual = 5;
    onetest_assert(*(int*)bufferGet(buf, 0) == actual);

    bufferFree(buf);
    return 0;
}

static int test_bufferPeek(void) {
    Aria_Buffer buf = bufferCreate(sizeof(int), 8);
    int* num = (int*)malloc(sizeof(int));
    *num = 1;
    bufferInsert(&buf, num);

    *num = 2;
    bufferInsert(&buf, num);

    *num = 3;
    bufferInsert(&buf, num);

    onetest_assert(*(int*)bufferPeek(buf) == 3);
    return 0;
}

static int test_bufferCopy(void) { return 1; }
