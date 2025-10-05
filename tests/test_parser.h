#include <string.h>
#include "onetest.h"

#include "aria_lexer.h"
#include "aria_parser.h"

static int test_parseConst(void) { return 1; }
static int test_parseFor(void) { return 1; }
static int test_parseIf(void) { return 1; }
static int test_parsePrint(void) { return 1; }
static int test_parseReturn(void) {
    Aria_Lexer* lexer = ariaTokenize("return 69;");
    ASTNode node = parseReturn(lexer);

    onetest_assert(node.type == AST_VALUE);
    onetest_assert(node.value == 69);

    return 0;
}

static int test_parseSwitch(void) { return 1; }
static int test_parseVar(void) { return 1; }
static int test_parseIdentifier(void) { return 1; }
static int test_parseClass(void) { return 1; }
static int test_parseExport(void) { return 1; }
static int test_parseBlock(void) {
    Aria_Lexer* lexer = ariaTokenize("{ return 800; return 85; }");
    ASTNode node = parseBlock(lexer);

    onetest_assert(node.type == AST_BLOCK);
    onetest_assert(node.block.buf->size == 2);

    ASTNode last_node = *(ASTNode*)bufferPeek(node.block.buf);
    onetest_assert(last_node.type == AST_VALUE);
    onetest_assert(last_node.value == 85);

    return 0;
}

static int test_parseFunc(void) {
    Aria_Lexer* lexer = ariaTokenize("func foo() { return 42; }");
    ASTNode node = parseFunc(lexer);

    onetest_assert(node.type == AST_FUNC);
    onetest_assert(strcmp(node.func.func_name, "foo") == 0);

    ASTNode body = *node.func.body;
    onetest_assert(body.type == AST_BLOCK);
    onetest_assert(body.block.buf->size == 1);

    ASTNode last_node = *(ASTNode*)bufferPeek(body.block.buf);
    onetest_assert(last_node.type == AST_VALUE);
    onetest_assert(last_node.value == 42);

    return 0;
}

static int test_parseImport(void) { return 1; }
static int test_createNode(void) {
    ASTNode node = createNode(AST_FUNC);
    onetest_assert(node.type = AST_FUNC);
    onetest_assert(node.func.func_name == NULL);
    return 0;
}
