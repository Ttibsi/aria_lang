#define ARIA_BUFFER_IMPL
#include "aria.h"

#include "aria_lexer.h"
#include "aria_parser.h"

[[nodiscard]] char* ariaVersion() {
    return "0.0.0";
}

void ariaFree(Aria_Lexer* L, ASTNode* ast) {
    // Lexer
    free(L->source);
    bufferFree(L->tokens);
    L = NULL;

    nodeFree(*ast);
}

[[nodiscard]] int ariaInterpret(char* text, const bool debug_mode) {
    Aria_Lexer L = lexerInit(text);
    ariaTokenize(&L);
    if (debug_mode) { printTokens(&L); }

    ASTNode ast = ariaParse(&L);
    if (debug_mode) { printAST(ast, &L); }

    ariaFree(&L, &ast);
    return 0;
}
