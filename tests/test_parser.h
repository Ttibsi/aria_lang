#include <string.h>
#include "onetest.h"
#include "aria_parser.h"

static inline int test_parseConst(void) { return 1; }
static inline int test_parseFor(void) { return 1; }
static inline int test_parseIf(void) { return 1; }
static inline int test_parseReturn(void) {
    Aria_Lexer L = lexerInit("return 69;");
    ariaTokenize(&L);
    const ASTNode node = parseReturn(&L);

    onetest_assert(node.type == AST_RETURN);
    onetest_assert(node.ret.expr->expr.lhs->value == 69);

    return 0;
}

static inline int test_parseSwitch(void) { return 1; }
static inline int test_parseVar(void) { return 1; }

static inline int test_parseFuncCall(void) {
    Aria_Lexer L = lexerInit("foo(1)");
    ariaTokenize(&L);
    advance(&L);
    onetest_assert(L.buf_index == 1);
    const ASTNode node = parseFuncCall(&L);

    onetest_assert(node.type == AST_FUNCCALL);
    onetest_assert(strcmp(node.func_call.func_name, "foo") == 0);
    onetest_assert(node.func_call.param_list[0]->type == TOK_NUMBER);

    return 0;
}

static inline int test_parseIdentifier(void) {
    Aria_Lexer L = lexerInit("foo(x)");
    ariaTokenize(&L);
    const ASTNode node = parseIdentifier(&L);

    onetest_assert(node.type == AST_FUNCCALL);
    onetest_assert(strcmp(node.func_call.func_name, "foo") == 0);
    onetest_assert(node.func_call.param_list[0]->type == TOK_IDENTIFIER);

    return 0;
}

static inline int test_parseExpression(void) {
    Aria_Lexer L = lexerInit("12 + (2 * 3);");
    ariaTokenize(&L);
    const ASTNode node = parseExpression(&L, 0);

    onetest_assert(node.type == AST_EXPR);
    onetest_assert(node.expr.op == TOK_PLUS);

    const ASTNode* lhs = node.expr.lhs;
    const ASTNode* rhs = node.expr.rhs;

    onetest_assert(lhs->type == AST_VALUE);
    onetest_assert(lhs->value == 12);

    onetest_assert(rhs->expr.op == TOK_STAR);
    onetest_assert(rhs->expr.lhs->value == 2);
    onetest_assert(rhs->expr.rhs->value == 3);

    return 0;
}

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
    onetest_assert(last_node.type == AST_RETURN);
    onetest_assert(last_node.ret.expr->expr.lhs->value == 85);

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
    onetest_assert(last_node.type == AST_RETURN);
    onetest_assert(last_node.ret.expr->expr.lhs->value == 42);

    return 0;
}

static inline int test_ariaParse(void) {
    Aria_Lexer L = lexerInit("func foo() { return 42; }");
    ariaTokenize(&L);
    ASTNode mod = ariaParse(&L);
    onetest_assert(mod.type == AST_MODULE);

    ASTNode* func = (ASTNode*)bufferGet(mod.block, 0);
    onetest_assert(func->type == AST_FUNC);
    onetest_assert(strcmp(func->func.func_name, "foo") == 0);

    ASTNode* body = func->func.body;
    onetest_assert(body->type == AST_BLOCK);
    onetest_assert(body->block->size == 1);

    return 0;
}

static inline int test_createNode(void) {
    ASTNode node = createNode(AST_FUNC);
    onetest_assert(node.type = AST_FUNC);
    onetest_assert(node.func.func_name == NULL);
    return 0;
}
