#ifndef ARIA_BUFFER_H
#define ARIA_BUFFER_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint32_t size;
    uint32_t capacity;
    uint32_t elem_size;
    void* items;
} Aria_Buffer;

static Aria_Buffer bufferCreate(uint32_t elem_size, uint32_t cap);
static void bufferFree(Aria_Buffer buf);
static void bufferInsert(Aria_Buffer* buf, void* elem);
static void* bufferGet(Aria_Buffer buf, uint32_t idx);
static void* bufferPeek(Aria_Buffer buf);

#ifdef ARIA_BUFFER_IMPL

static Aria_Buffer bufferCreate(uint32_t elem_size, uint32_t cap) {
    Aria_Buffer buf = {
        .size = 0,
        .capacity = cap,
        .elem_size = elem_size,
        .items = malloc(elem_size * cap)
    };

    return buf;
}

static void bufferFree(Aria_Buffer buf) {
    free(buf.items);
    buf.capacity = 0;
}

static void bufferInsert(Aria_Buffer* buf, void* elem) {
    if (buf->size == buf->capacity) {
        buf->capacity *= 2;
        realloc(buf->items, buf->capacity);
    }

    memcpy(buf->items + (buf->elem_size * buf->size), elem, buf->elem_size);
    buf->size++;
}

static void* bufferGet(Aria_Buffer buf, uint32_t idx) {
    if (idx > buf.size) { return NULL; }
    return buf.items + (buf.elem_size * idx);
}

static void* bufferPeek(Aria_Buffer buf) {
    if (buf.size == 0) { return NULL; }
    return buf.items + (buf.elem_size * (buf.size - 1));
}

#endif // ARIA_BUFFER_IMPL
#endif // ARIA_BUFFER_H
