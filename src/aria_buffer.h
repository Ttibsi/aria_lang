#ifndef ARIA_BUFFER_H
#define ARIA_BUFFER_H

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
}

void bufferFree(Aria_Buffer buf) {
}

void bufferInsert(Aria_Buffer buf, void* elem) {
}

void* bufferGet(Aria_Buffer buf, uint32_t idx) {
}

#endif // ARIA_BUFFER_IMPL
#endif // ARIA_BUFFER_H
