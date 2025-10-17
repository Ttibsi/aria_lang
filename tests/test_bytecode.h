#include <string.h>

#include "onetest.h"
#include "aria_bytecode.h"

static inline int test_compileFunc(void) {
    Aria_Lexer L = lexerInit("func foo() { return 42; }");
    ariaTokenize(&L);
    const ASTNode mod = ariaParse(&L);

    ASTNode* funcNode = (ASTNode*)bufferGet(mod.block, 0);
    onetest_assert(funcNode->type == AST_FUNC);

    const Aria_Chunk* chunk = compileFunc(funcNode);
    onetest_assert(strcmp(chunk->name, "foo") == 0);
    onetest_assert(chunk->buf->op == OP_STORE_CONST);
    onetest_assert(chunk->buf->operand == 42);

    return 0;
}

static inline int test_ariaCompile(void) {
    Aria_Lexer L = lexerInit("func foo(bar) { return 42; } func main(argv) { return 69; }");
    ariaTokenize(&L);
    const ASTNode ast = ariaParse(&L);
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

    return 0;
}

static inline int test_opcodeDisplay(void) {
    onetest_assert(strcmp(opcodeDisplay(OP_STORE_CONST), "OP_STORE_CONST") == 0);
    onetest_assert(strcmp(opcodeDisplay(AST_FUNC), "UNKNOWN") == 0);
    return 0;
}
