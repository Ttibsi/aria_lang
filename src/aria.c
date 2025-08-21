#include "aria.h"

#include <stdio.h>
#include <stdlib.h>

// TODO: Implement a hash table here from crafting interpreters for variable assignment

int aria_interpret(const char* name, const char* src) {
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
    
    // Parse into AST
    ParserState* state = malloc(sizeof(ParserState));
    state->prev = NULL;
    state->curr = NULL;
    state->next = malloc(sizeof(Aria_Token));
    *state->next = scanToken(&lexer);
    advance_state(state, &lexer);

    Expression expr = parse_expression(state, &lexer, 0.0); 
    
#ifdef ARIA_DEBUG
    printf("\n=== AST ===\n"); 
    print_exprs(expr);
#endif

    // eval here
    // cleanup here

    return 0;
}

