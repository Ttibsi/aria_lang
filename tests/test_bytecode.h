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

    freeLexer(lexer);
    return 0;
}

static int test_opcodeDisplay(void) {
    onetest_assert(strcmp(opcodeDisplay(OP_STORE_CONST), "OP_STORE_CONST") == 0);
    onetest_assert(strcmp(opcodeDisplay(AST_FUNC), "UNKNOWN") == 0);
    return 0;
}

static int test_ariaCompile(void) {
    Aria_Lexer* lexer = ariaTokenize("func foo(bar) { return 42; } func main(argv) { return 69; }");
    ASTNode ast = ariaParse(lexer);
    Aria_Module* mod = ariaCompile(&ast);

    onetest_assert(strcmp(mod->name, "main") == 0);
    onetest_assert(mod->buf->size == 2);

    Aria_Chunk* first_chunk = bufferGet(mod->buf, 0);
    onetest_assert(strcmp(first_chunk->name, "foo") == 0);
    onetest_assert(first_chunk->buf->op == OP_STORE_CONST);
    onetest_assert(first_chunk->buf->operand == 42);

    Aria_Chunk* second_chunk = bufferGet(mod->buf, 1);
    onetest_assert(strcmp(second_chunk->name, "main") == 0);
    onetest_assert(second_chunk->buf->op == OP_STORE_CONST);
    onetest_assert(second_chunk->buf->operand == 69);

    freeLexer(lexer);
    return 0;
}
