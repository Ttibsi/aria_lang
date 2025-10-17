#include <string.h>
#include "onetest.h"
#include "aria_parser.h"

static inline int test_parseConst(void) { return 1; }
static inline int test_parseFor(void) { return 1; }
static inline int test_parseIf(void) { return 1; }
static inline int test_parsePrint(void) { return 1; }
static inline int test_parseReturn(void) {
    Aria_Lexer L = lexerInit("return 69;");
    ariaTokenize(&L);
    const ASTNode node = parseReturn(&L);

    onetest_assert(node.type == AST_VALUE);
    onetest_assert(node.value == 69);

    return 0;
}

static inline int test_parseSwitch(void) { return 1; }
static inline int test_parseVar(void) { return 1; }
static inline int test_parseIdentifier(void) { return 1; }
static inline int test_parseExpression(void) { return 1; }
static inline int test_parseClass(void) { return 1; }
static inline int test_parseExport(void) { return 1; }
static inline int test_parseImport(void) { return 1; }
static inline int test_parseBlock(void) {
    Aria_Lexer L = lexerInit("{ return 800; return 85; }");
    ariaTokenize(&L);
    const ASTNode node = parseBlock(&L);

    onetest_assert(node.type == AST_BLOCK);
    onetest_assert(node.block->size == 2);

    const ASTNode last_node = *(ASTNode*)bufferPeek(node.block);
    onetest_assert(last_node.type == AST_VALUE);
    onetest_assert(last_node.value == 85);

    return 0;
}

static inline int test_parseFunc(void) {
    Aria_Lexer L = lexerInit("func foo() { return 42; }");
    ariaTokenize(&L);
    const ASTNode node = parseFunc(&L);

    onetest_assert(node.type == AST_FUNC);
    onetest_assert(strcmp(node.func.func_name, "foo") == 0);

    const ASTNode body = *node.func.body;
    onetest_assert(body.type == AST_BLOCK);
    onetest_assert(body.block->size == 1);

    const ASTNode last_node = *(ASTNode*)bufferPeek(body.block);
    onetest_assert(last_node.type == AST_VALUE);
    onetest_assert(last_node.value == 42);

    return 0;
}

static inline int test_ariaParse(void) {
    Aria_Lexer L = lexerInit("func foo() { return 42; }");
    ariaTokenize(&L);
    const ASTNode node = ariaParse(&L);

    onetest_assert(node.type == AST_FUNC);
    onetest_assert(strcmp(node.func.func_name, "foo") == 0);

    const ASTNode body = *node.func.body;
    onetest_assert(body.type == AST_BLOCK);
    onetest_assert(body.block->size == 1);

    const ASTNode last_node = *(ASTNode*)bufferPeek(body.block);
    onetest_assert(last_node.type == AST_VALUE);
    onetest_assert(last_node.value == 42);

    return 0;
}

static inline int test_createNode(void) {
    ASTNode node = createNode(AST_FUNC);
    onetest_assert(node.type = AST_FUNC);
    onetest_assert(node.func.func_name == NULL);
    return 0;
}
