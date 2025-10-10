#include "aria.h"

#include <stdio.h>

#include "aria_lexer.h"
#include "aria_parser.h"
#include "aria_bytecode.h"
#include "aria_executor.h"
#define ARIA_STACK_IMPL
#include "aria_stack.h"

int aria_debug_mode;

int ariaInterpret(char* src) {
    Aria_Lexer* lexer = ariaTokenize(src);
    if (aria_debug_mode) { printTokens(lexer); }

    ASTNode ast = ariaParse(lexer);

    if (aria_debug_mode) {
        printf("\n=== AST ===\n");
        printAST(ast, 0, lexer);
    }

    freeLexer(lexer);

    Aria_Module* main_mod = ariaCompile(&ast);

    if (aria_debug_mode) {
        printf("\n=== BYTECODE ===\n");
        printModule(main_mod);
    }

    // execute bytecode
    Stack* stack = ariaExecute(main_mod);

    if (isEmpty(stack)) { stackPush(stack, 1); }
    int result = stackPeek(stack);

    // cleanup here
    //freeStack(stack);
    //freeBytecode(bc);

    return result;
}
