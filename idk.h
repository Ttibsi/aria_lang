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
    int start;
    int len;
} Aria_Token; 
 
typedef struct {
    Aria_Token* data;
    int size;
    int capacity;
    const char* source; // null-terminated
    const char* module; // null-terminated
} Aria_Lexer;

typedef struct {
    Aria_Lexer* lexer; 
} Aria_VM; 

// Global
Aria_VM aria_vm_init();
void aria_vm_destroy(Aria_VM* aria_vm);
void aria_interpret(Aria_VM* aria_vm, const char* module, const char* source);

// Lexer
Aria_Lexer aria_tokenize(Aria_VM* vm, const char* module, const char* source);
static void aria_lexer_append(Aria_Lexer* lexer, Aria_Token* tok);

#endif // ARIA_H 
