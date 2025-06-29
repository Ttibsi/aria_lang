#include "idk.h"

#include <string.h> 

void aria_vm_init(Aria_VM* aria_vm, char* in_str, int length) {
    aria_vm->arena = arena_init(16 *1024); 
}

void aria_vm_destroy(Aria_VM* aria_vm) {
}

void aria_interpret(Aria_VM* aria_vm, const char* module, const char* source) {
} 
 
Aria_arena arena_init(size_t capacity) {
    void* data = malloc(sizeof(uint8_t) * capacity);
    if (data == NULL) {
        fprintf(stderr, "ERR: buy more ram");
        exit(1);
    }
 
    Aria_arena arena = {
        .next = NULL,
        .capacity = capacity,
        .size = 0,
        .data = data 
    };
} 

void* arena_alloc(Aria_arena* arena, size_t size) {
    Aria_arena* cur = arena; 
    while (!(cur->size + size <= cur->capacity)) {
        if (cur->next == NULL) {
            Aria_arena* next = malloc(sizeof(Aria_arena));
            Aria_arena init = arena_init(arena->capacity + size);
            memcpy(next, &init, sizeof(Aria_arena));
            cur->next = next;
        }
        cur = cur->next;
    }

    uint8_t* data = &cur->data[cur->size];
    cur->size += size;
    return data; 
} 

void arena_free(Aria_arena* arena) {
    free(arena->data);
    arena->capacity = 0;
    arena->size = 0;

    Aria_arena* cur = arena->next; 
    while (cur != NULL) { 
        Aria_arena* tmp = cur->next;
        free(cur->data);
        free(cur);
        cur = tmp; 
    } 

    arena->next = NULL;
} 
