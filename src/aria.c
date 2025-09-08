#include "aria.h"

#include <stdio.h>
#include <stdlib.h>

#include "aria_lexer.h"
#include "aria_parser.h"
#include "aria_bytecode.h"
#include "aria_executor.h"

int aria_debug_mode;

// TODO: Implement a hash table here from crafting interpreters for variable assignment

int ariaInterpret(const char* src) {
    Aria_Lexer lexer = {src, 0, {false, TOK_EOF, 0, 0}};

    if (aria_debug_mode) {
        printf("=== TOKENS ===\n");
        Aria_Token token;
        do {
            token = scanToken(&lexer);
            printf("Token: %d, start: %d, len: %d\n", token.type, token.start, token.len);
        } while (token.type != TOK_EOF);

        // Reset lexer for parsing
        lexer.pc = 0;
    }

    // Parse into AST
    ParserState* state = malloc(sizeof(ParserState));
    state->prev = NULL;
    state->curr = NULL;
    state->next = malloc(sizeof(Aria_Token));
    *state->next = scanToken(&lexer);
    advanceState(state, &lexer);

    Expression expr = parseExpression(state, &lexer, 0.0);

    if (aria_debug_mode) {
        printf("\n=== AST ===\n");
        printExprs(expr);
        printf("\n");
    }

    // Convert AST to bytecode
    Bytecode* bc = bytecodeGeneration(expr);
    if (aria_debug_mode) {
        printBytecode(bc);
        printf("\n");
    }

    // execute bytecode
    Stack* stack = ariaExecute(bc);
    // int result = stackPeek(stack);

    // cleanup here
    freeStack(stack);
    freeBytecode(bc);

    return 0;
}
