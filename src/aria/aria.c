#define ARIA_BUFFER_IMPL
#include "aria.h"

[[nodiscard]] char* ariaVersion() {
    return "0.0.0";
}

void ariaFree(Aria_Lexer* L) {
    // Lexer
    free(L->source);
    bufferFree(L->tokens);
    L = NULL;
}

[[nodiscard]] int ariaInterpret(char* text, const bool debug_mode) {
    Aria_Lexer L = lexerInit(text);
    ariaTokenize(&L);
    if (debug_mode) { printTokens(&L); }

    ariaFree(&L);
    return 0;
}

