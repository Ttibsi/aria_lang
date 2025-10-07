#include <stdlib.h>

#include "onetest.h"
#include "aria_bytecode.h"

static int test_compileFunc(void) {
    Aria_Lexer* lexer = ariaTokenize("func foo() { return 42; }");
    ASTNode mod = ariaParse(lexer);
    ASTNode *funcNode = (ASTNode *)bufferGet(mod.block.buf, 0);
    if (funcNode->type != AST_FUNC) {
        fprintf(stderr, "Expected a function node\n");
        return 1;
    }

    Aria_Chunk* chunk = compileFunc(funcNode);

    onetest_assert(strcmp(chunk->name, "foo") == 0);
    onetest_assert(chunk->buf->op == OP_STORE_CONST);
    onetest_assert(chunk->buf->operand == 42);

    return 0;
}

static int test_opcodeDisplay(void) {
    onetest_assert(strcmp(opcodeDisplay(OP_STORE_CONST), "OP_STORE_CONST") == 0);
    onetest_assert(strcmp(opcodeDisplay(AST_FUNC), "UNKNOWN") == 0);
    return 0;
}

static int test_ariaCompile(void) { return 1; }
