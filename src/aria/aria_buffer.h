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

[[maybe_unused]] Aria_Buffer* bufferCreate(uint32_t elem_size, uint32_t cap);
[[maybe_unused]] void bufferFree(Aria_Buffer* buf);
[[maybe_unused]] void bufferInsert(Aria_Buffer* buf, void* elem);
[[maybe_unused]] void* bufferGet(const Aria_Buffer* buf, uint32_t idx);
[[maybe_unused]] void* bufferPeek(const Aria_Buffer* buf);
[[maybe_unused]] void bufferCopy(Aria_Buffer* dst, const Aria_Buffer* src);

#ifdef ARIA_BUFFER_IMPL

[[maybe_unused]] Aria_Buffer* bufferCreate(uint32_t elem_size, uint32_t cap) {
    Aria_Buffer* buf = malloc(sizeof(Aria_Buffer));
    buf->size = 0;
    buf->capacity = cap;
    buf->elem_size = elem_size;
    buf->items = malloc(elem_size * cap);

    return buf;
}

[[maybe_unused]] void bufferFree(Aria_Buffer* buf) {
    free(buf->items);
    buf->capacity = 0;
}

[[maybe_unused]] void bufferInsert(Aria_Buffer* buf, void* elem) {
    if (buf->size == buf->capacity) {
        buf->capacity *= 2;
        buf->items = realloc(buf->items, buf->capacity * buf->elem_size);
    }

    memcpy(buf->items + (buf->elem_size * buf->size), elem, buf->elem_size);
    buf->size++;
}

[[maybe_unused]] void* bufferGet(const Aria_Buffer* buf, uint32_t idx) {
    if (idx > buf->size) { return NULL; }
    return buf->items + (buf->elem_size * idx);
}

[[maybe_unused]] void* bufferPeek(const Aria_Buffer* buf) {
    if (buf->size == 0) { return NULL; }
    return buf->items + (buf->elem_size * (buf->size - 1));
}

[[maybe_unused]] void bufferCopy(Aria_Buffer* dst, const Aria_Buffer* src) {
    // To prevent copying an empty buffer
    if (src == NULL) { return; }

    dst->size = src->size;
    dst->capacity = src->capacity;
    dst->elem_size = src->elem_size;

    if (src->items) {
        dst->items = malloc(src->capacity * src->elem_size);
        memcpy(dst->items, src->items, src->size * src->elem_size);
    } else {
        dst->items = NULL;
    }
}

#endif // ARIA_BUFFER_IMPL
#endif // ARIA_BUFFER_H
