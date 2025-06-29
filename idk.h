#ifndef ARIA_H
#define ARIA_H

#include <stdint.h> 

typedef enum {
} TokenType;
 
typedef struct {
    TokenType type; 
} Token;
 
typedef struct {
    char* input_text;
    int length;
} Aria_Lexer;

// Arena inspired by: cobbcoding1/c-allocators 
typedef struct {
    Aria_arena* next;
    size_t capacity;
    size_t size;
    uint8_t data; 
} Aria_arena; 
 
typedef struct {
    Aria_arena arena; 
    Aria_Lexer* lexer; 
} Aria_VM; 

Aria_arena arena_init(size_t capacity) {
    void* data = malloc(sizeof(uint8_t) * capacity);
    if (data == NULL) {
        fprintf("ERR: buy more ram");
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
            Aria_arena init = arena_init(arena->capacty + size);
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

    Aria_arena cur = arena->next; 
    while (cur != NULL) { 
        Aria_arena* tmp = cur->next;
        free(cur->data);
        free(cur);
        cur = tmp; 
    } 

    arena->next = NULL;
} 

#endif // ARIA_H 
