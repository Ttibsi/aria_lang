#ifndef ARIA_BUFFER_H
#define ARIA_BUFFER_H

#include <stdint.h>
#include <stdlib.h>

typedef struct {
    uint32_t size;
    uint32_t capacity;
    uint32_t elem_size;
    void* items;
} Aria_Buffer;

Aria_Buffer bufferCreate(uint32_t elem_size, uint32_t cap);
void bufferFree(Aria_Buffer buf);
void bufferInsert(Aria_Buffer buf, void* elem);
void* bufferGet(Aria_Buffer buf, uint32_t idx);

#ifdef ARIA_BUFFER_IMPL

Aria_Buffer bufferCreate(uint32_t elem_size, uint32_t cap) {
    Aria_Buffer buf = {
        .size = 0,
        .capacity = cap,
        .elem_size = elem_size,
        .items = malloc(elem_size * cap)
    };

    return buf;
}

void bufferFree(Aria_Buffer buf) {
    free(buf.items);
    buf.capacity = 0;
}

void bufferInsert(Aria_Buffer buf, void* elem) {
    if (buf.size == buf.capacity) {
        buf.capacity *= 2;
        realloc(buf.items, buf.capacity);
    }

    buf.items[buf.size] = elem;
    buf.size++;
}

void* bufferGet(Aria_Buffer buf, uint32_t idx) {
    if (idx > buf.size) { return NULL; }
    return buf[idx];
}

#endif // ARIA_BUFFER_IMPL
#endif // ARIA_BUFFER_H
