#include "aria.h"

#include <stdio.h>

#include "aria_lexer.h"
#include "aria_parser2.h"
// #include "aria_bytecode.h"
// #include "aria_executor.h"

int aria_debug_mode;

int ariaInterpret(const char* src) {
    Aria_Lexer lexer = ariaTokenize(src);
    if (aria_debug_mode) { printTokens(&lexer); }

    ASTNode ast = ariaParse(&lexer);

    if (aria_debug_mode) {
        printf("\n=== AST ===\n");
        printAST(ast, 0, &lexer);
        printf("\n");
    }

    return 0;

    // // Convert AST to bytecode
    // Bytecode* bc = bytecodeGeneration(expr);
    // if (aria_debug_mode) {
    //     printBytecode(bc);
    //     printf("\n");
    // }
    //
    // // execute bytecode
    // Stack* stack = ariaExecute(bc);
    //
    // if (isEmpty(stack)) { stackPush(stack, 0); }
    // int result = stackPeek(stack);
    //
    // // cleanup here
    // freeStack(stack);
    // freeBytecode(bc);

    // return result;
}
