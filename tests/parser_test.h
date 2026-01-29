#include <string.h>

#include "aria/aria_parser.h"
#include "onetest.h"

static inline int test_parseArg(void) {
    AriaLexer L = {0};
    ariaLexerInit(&L, "foo LIST[BOOL]");
    ariaTokenize(&L);

    ASTNode* n = parseArg(&L);
    onetest_assert(n->type == AST_ARG);

    onetest_assert(strcmp(n->arg.name, "foo") == 0);
    onetest_assert(n->arg.type == TOK_LIST);
    onetest_assert(n->arg.inner_type_1 == TOK_BOOL);

    // 0th element, nothing set here
    onetest_assert(n->arg.inner_type_2 == TOK_LEFT_PAREN);

    return 0;
}

static inline int test_parseAssignment(void) {
    AriaLexer L = {0};
    ariaLexerInit(&L, "foo = 2 + 3 END");
    ariaTokenize(&L);

    ASTNode n = parseAssignment(&L);
    onetest_assert(n.type == AST_ASSIGN);

    onetest_assert(n.assign.ident->type == AST_IDENT);
    onetest_assert(strcmp(n.assign.ident->identifier, "foo") == 0);

    onetest_assert(n.assign.expr->type == AST_EXPR);
    onetest_assert(n.assign.expr->expr.op == TOK_PLUS);

    return 0;
}

static inline int test_parseBlock(void) {
    AriaLexer L = {0};
    ariaLexerInit(&L, "VAR x NUM = 5 VAR y NUM = 2 END");
    ariaTokenize(&L);

    ASTNode n = parseBlock(&L);
    onetest_assert(n.type == AST_BLOCK);

    onetest_assert(n.block.count == 2);

    return 0;
}

static inline int test_parseFunc(void) {
    AriaLexer L = {0};
    ariaLexerInit(&L, "FUNC f(arg NUM) NUM RET 0 END");
    ariaTokenize(&L);

    ASTNode n = parseFunc(&L);
    onetest_assert(n.type == AST_FUNC);

    onetest_assert(strcmp(n.func.name, "f") == 0);

    onetest_assert(strcmp(n.func.args[0]->arg.name, "arg") == 0);
    onetest_assert(n.func.args[1] == 0);
    onetest_assert(n.func.ret_type == TOK_NUM);

    return 0;
}

static inline int test_parseFuncCall(void) {
    AriaLexer L = {0};
    ariaLexerInit(&L, "f(foo, bar)");
    ariaTokenize(&L);

    ASTNode n = parseFuncCall(&L);
    onetest_assert(n.type == AST_CALL);

    onetest_assert(strcmp(n.funcCall.name, "f") == 0);
    onetest_assert(strcmp(n.funcCall.args[0], "foo") == 0);
    onetest_assert(strcmp(n.funcCall.args[1], "bar") == 0);

    return 0;
}

static inline int test_parseExpression(void) {
    AriaLexer L = {0};
    ariaLexerInit(&L, "(17 * 2) + 35 END");
    ariaTokenize(&L);

    ASTNode n = parseExpression(&L, 0);
    onetest_assert(n.type == AST_EXPR);

    onetest_assert(n.expr.lhs != NULL);
    onetest_assert(n.expr.rhs != NULL);
    onetest_assert(n.expr.op == TOK_PLUS);

    ASTNode* lhs = n.expr.lhs;
    ASTNode* rhs = n.expr.rhs;

    onetest_assert(lhs->type == AST_EXPR);
    onetest_assert(lhs->expr.lhs->type == AST_NUM_LIT);
    onetest_assert(lhs->expr.lhs->num_literal == 17);
    onetest_assert(lhs->expr.rhs->type == AST_NUM_LIT);
    onetest_assert(lhs->expr.rhs->num_literal == 2);
    onetest_assert(lhs->expr.op == TOK_STAR);

    onetest_assert(rhs->type == AST_NUM_LIT);
    onetest_assert(rhs->num_literal == 35);

    return 0;
}

static inline int test_parseFor(void) { return 1; }

static inline int test_parseForEach(void) { return 1; }

static inline int test_parseIdentifier(void) { return 1; }

static inline int test_parseIf(void) { return 1; }

static inline int test_parseImport(void) { return 1; }

static inline int test_parseReturn(void) { return 1; }

static inline int test_parseMethodCall(void) { return 1; }

static inline int test_parseMethodCallOrAttr(void) { return 1; }

static inline int test_parseStatement(void) { return 1; }

static inline int test_parseType(void) { return 1; }

static inline int test_parseVar(void) { return 1; }

static inline int test_ariaCreateNode(void) { return 1; }

static inline int test_ariaParse(void) { return 1; }
