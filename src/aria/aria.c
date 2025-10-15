#define ARIA_BUFFER_IMPL
#include "aria.h"

#include "aria_lexer.h"
#include "aria_parser.h"
#include "aria_bytecode.h"

[[nodiscard]] char* ariaVersion() {
    return "0.0.0";
}

void ariaFree(Aria_Lexer* L, ASTNode* ast, Aria_Module* mod) {
    // Lexer
    free(L->source);
    bufferFree(L->tokens);
    L = NULL;

    // Parser
    nodeFree(*ast);
    ast = NULL;

    // Bytecode
    for (size_t i = 0; i < mod->buf->size; ++i) {
        Aria_Chunk *c = bufferGet(mod->buf, i);
        freeChunk(c);
    }
    bufferFree(mod->buf);
    free(mod);
    mod = NULL;
}

[[nodiscard]] int ariaInterpret(char* text, const bool debug_mode) {
    Aria_Lexer L = lexerInit(text);
    ariaTokenize(&L);
    if (debug_mode) { printTokens(&L); }

    ASTNode ast = ariaParse(&L);
    if (debug_mode) { printAST(ast, &L); }

    Aria_Module* main_mod = ariaCompile(&ast);
    if (debug_mode) { printBytecode(main_mod); }

    ariaFree(&L, &ast, main_mod);
    return 0;
}
