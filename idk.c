#include "idk.h"

#include <string.h> 

Aria_VM aria_vm_init() {
    Aria_VM vm = {
        .lexer = NULL
    };

    return vm;
}

void aria_vm_destroy(Aria_VM* aria_vm) {
}

void aria_interpret(Aria_VM* aria_vm, const char* module, const char* source) {
    *aria_vm->lexer = aria_tokenize(aria_vm, module, source);
} 
 
#define TOKEN_APPEND(type, start, len, l) \
do { \
    Aria_Token t = {.type = type, .start = start, .len = len}; \
    start += len; \
    aria_lexer_append(&l, &t); \
} while (0)

Aria_Lexer aria_tokenize(Aria_VM* vm, const char* module, const char* source) {
    Aria_Lexer lexer = {
        .data = malloc(32 * sizeof(Aria_Token)),
        .size = 0,
        .capacity = 32,
        .source = source,
        .module = module
    };

    char* current = source;
    int start = 0;

    while (*current != '\0') {
        switch (*current) {
            case ' ': TOKEN_APPEND();
        }
    }

    return lexer;
} 

static void aria_lexer_append(Aria_Lexer* lexer, Aria_Token* tok) {
    ...
}
