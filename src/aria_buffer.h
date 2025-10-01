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

[[maybe_unused]] static Aria_Buffer bufferCreate(uint32_t elem_size, uint32_t cap);
[[maybe_unused]] static void bufferFree(Aria_Buffer buf);
[[maybe_unused]] static void bufferInsert(Aria_Buffer* buf, void* elem);
[[maybe_unused]] static void* bufferGet(Aria_Buffer buf, uint32_t idx);
[[maybe_unused]] static void* bufferPeek(Aria_Buffer buf);

#ifdef ARIA_BUFFER_IMPL

[[maybe_unused]] static Aria_Buffer bufferCreate(uint32_t elem_size, uint32_t cap) {
    Aria_Buffer buf = {
        .size = 0,
        .capacity = cap,
        .elem_size = elem_size,
        .items = malloc(elem_size * cap)
    };

    return buf;
}

[[maybe_unused]] static void bufferFree(Aria_Buffer buf) {
    free(buf.items);
    buf.capacity = 0;
}

[[maybe_unused]] static void bufferInsert(Aria_Buffer* buf, void* elem) {
    if (buf->size == buf->capacity) {
        buf->capacity *= 2;
        buf->items = realloc(buf->items, buf->capacity * buf->elem_size);
    }

    void* copy_elem = malloc(buf->elem_size);
    memcpy(copy_elem, elem, buf->elem_size);

    memcpy(buf->items + (buf->elem_size * buf->size), copy_elem, buf->elem_size);
    buf->size++;
}

[[maybe_unused]] static void* bufferGet(Aria_Buffer buf, uint32_t idx) {
    if (idx > buf.size) { return NULL; }
    return buf.items + (buf.elem_size * idx);
}

[[maybe_unused]] static void* bufferPeek(Aria_Buffer buf) {
    if (buf.size == 0) { return NULL; }
    return buf.items + (buf.elem_size * (buf.size - 1));
}

#endif // ARIA_BUFFER_IMPL
#endif // ARIA_BUFFER_H
