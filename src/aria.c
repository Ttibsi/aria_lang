#include "aria.h"

#include <stdio.h>

// TODO: Implement a hash table here from crafting interpreters for variable assignment

/// VM Interface
int aria_interpret(Aria_VM* vm, const char* name, const char* src) {
    Aria_Lexer lexer = {src, 0, {false, TOK_EOF, 0, 0}};

#ifdef ARIA_DEBUG
    printf("=== TOKENS ===\n"); 
    Aria_Token token;
    do {
        token = scanToken(&lexer);
        printf("Token: %d, start: %d, len: %d\n", token.type, token.start, token.len);
    } while (token.type != TOK_EOF);
    
    // Reset lexer for parsing
    lexer.pc = 0;
#endif

    // Initialize with first token
    advance(&lexer);
    
    // Parse into AST and store in VM
    vm->ast_root = parseExpression(&lexer);
    
#ifdef ARIA_DEBUG
    printf("\n=== AST ===\n"); 
    printAST(vm->ast_root, 0);
#endif

    // eval here
    // cleanup here

    return 0;
}

Aria_VM aria_vm_init(void) {
    return (Aria_VM){0};
}

void aria_vm_destroy(Aria_VM* vm) {
    // VM cleanup will be implemented later
}
