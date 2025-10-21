#define ARIA_BUFFER_IMPL
#define ARIA_STACK_IMPL

#include "aria.h"
#include "aria_lexer.h"
#include "aria_parser.h"
#include "aria_bytecode.h"
#include "aria_execute.h"
#include "aria_stack.h"

[[nodiscard]] char* ariaVersion() {
    return "0.0.0";
}

// Do we want this to be a user-exposed function instead? So we can
// free after the user uses the contents (ex interacts with the stack/adds custom functions?)
void ariaFree(Aria_Lexer* L, ASTNode* ast, Aria_Module* mod, Stack* stack) {
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
    bufferFree(mod->identifiers);
    free(mod);
    mod = NULL;

    // Stack
    freeStack(stack);
}

[[nodiscard]] int ariaInterpret(char* text, const bool debug_mode) {
    Aria_Lexer L = lexerInit(text);
    ariaTokenize(&L);
    if (debug_mode) { printTokens(&L); }

    ASTNode ast = ariaParse(&L);
    if (debug_mode) { printAST(ast, &L); }

    Aria_Module* main_mod = ariaCompile(&ast);
    if (debug_mode) { printBytecode(main_mod); }

    Stack* stack = ariaExecute(main_mod);
    if (isEmpty(stack)) { stackPush(stack, 1); }
    int result = stackPeek(stack);

    ariaFree(&L, &ast, main_mod, stack);
    return result;
}
