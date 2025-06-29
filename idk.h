#ifndef ARIA_H
#define ARIA_H

#include <stddef.h>
#include <stdint.h> 
#include <stdio.h>
#include <stdlib.h>

typedef enum {
    TOK_EOF,
} TokenType;
 
typedef struct {
    TokenType type; 
} Token;
 
typedef struct {
    char* input_text;
    int length;
} Aria_Lexer;

// Arena inspired by: cobbcoding1/c-allocators 
typedef struct Aria_arena {
    struct Aria_arena* next;
    size_t capacity;
    size_t size;
    uint8_t* data; 
} Aria_arena; 
 
typedef struct {
    Aria_arena arena; 
    Aria_Lexer* lexer; 
} Aria_VM; 

// Global
void aria_vm_init(Aria_VM* aria_vm, char* in_str, int length);
void aria_vm_destroy(Aria_VM* aria_vm);
void aria_interpret(Aria_VM* aria_vm, const char* module, const char* source);

// Arena -- Inspired by https://github.com/CobbCoding1/c-allocators
Aria_arena arena_init(size_t capacity);
void* arena_alloc(Aria_arena* arena, size_t size);
void arena_free(Aria_arena* arena);

// Lexer

#endif // ARIA_H 
