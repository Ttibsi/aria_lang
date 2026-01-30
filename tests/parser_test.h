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

static inline int test_parseFor(void) {
    AriaLexer L = {0};
    ariaLexerInit(&L, "FOR i = 0 TO 5 THEN ... END");
    ariaTokenize(&L);

    ASTNode n = parseFor(&L);
    onetest_assert(n.type == AST_FOR);

    onetest_assert(n.For.var->type == AST_IDENT);
    onetest_assert(strcmp(n.For.var->identifier, "i") == 0);

    onetest_assert(n.For.start->type == AST_NUM_LIT);
    onetest_assert(n.For.stop->type == AST_NUM_LIT);
    onetest_assert(n.For.step == NULL);
    onetest_assert(n.For.block->type == AST_BLOCK);

    return 0;
}

static inline int test_parseForEach(void) {
    AriaLexer L = {0};
    ariaLexerInit(&L, "FOREACH idx, elem IN myList THEN ... END");
    ariaTokenize(&L);

    ASTNode n = parseForEach(&L);
    onetest_assert(n.type == AST_FOREACH);

    onetest_assert(n.ForEach.first_var->type == AST_IDENT);
    onetest_assert(strcmp(n.ForEach.first_var->identifier, "idx") == 0);

    onetest_assert(n.ForEach.sec_var->type == AST_IDENT);
    onetest_assert(strcmp(n.ForEach.sec_var->identifier, "elem") == 0);

    onetest_assert(n.ForEach.container->type == AST_IDENT);
    onetest_assert(strcmp(n.ForEach.container->identifier, "myList") == 0);

    onetest_assert(n.ForEach.block->type == AST_BLOCK);

    return 0;
}

static inline int test_parseIdentifier(void) {
    AriaLexer L = {0};
    ariaLexerInit(&L, "\"hello world\"");
    ariaTokenize(&L);

    ASTNode n = parseIdentifier(&L);
    onetest_assert(n.type == AST_IDENT);
    onetest_assert(strcmp(n.identifier, "\"hello world\"") == 0);

    return 0;
}

static inline int test_parseIf(void) {
    AriaLexer L = {0};
    ariaLexerInit(&L, "IF 3 < 5 THEN ... ELSE ... END");
    ariaTokenize(&L);

    ASTNode n = parseIf(&L);
    onetest_assert(n.type == AST_IF);

    onetest_assert(n.If.cond->type == AST_EXPR);
    onetest_assert(n.If.block->type == AST_BLOCK);
    onetest_assert(n.If.elseBlock->type == AST_BLOCK);

    onetest_assert(n.If.cond->expr.lhs->num_literal == 3);
    onetest_assert(n.If.cond->expr.op == TOK_LESS);
    onetest_assert(n.If.cond->expr.rhs->num_literal == 5);

    return 0;
}

static inline int test_parseImport(void) {
    AriaLexer L = {0};
    ariaLexerInit(&L, "IMPORT foo");
    ariaTokenize(&L);

    ASTNode n = parseImport(&L);
    onetest_assert(n.type == AST_IMPORT);

    onetest_assert(n.import.local_file == false);
    onetest_assert(strcmp(n.import.name, "foo") == 0);

    return 0;
}

static inline int test_parseReturn(void) {
    AriaLexer L = {0};
    ariaLexerInit(&L, "RET 4 END");
    ariaTokenize(&L);

    ASTNode n = parseReturn(&L);
    onetest_assert(n.type == AST_RETURN);

    onetest_assert(n.ret.expr != NULL);
    onetest_assert(n.ret.expr->type == AST_NUM_LIT);
    onetest_assert(n.ret.expr->num_literal == 4);

    return 0;
}

static inline int test_parseMethodCall(void) {
    AriaLexer L = {0};
    ariaLexerInit(&L, "object.method(arg)");
    ariaTokenize(&L);

    ASTNode n = parseMethodCall(&L);
    onetest_assert(n.type == AST_METHOD_CALL);

    onetest_assert(strcmp(n.methodCall.object, "object") == 0);
    onetest_assert(n.methodCall.method != NULL);

    onetest_assert(n.methodCall.method->type == AST_CALL);
    onetest_assert(strcmp(n.methodCall.method->funcCall.name, "method") == 0);
    onetest_assert(strcmp(n.methodCall.method->funcCall.args[0], "arg") == 0);

    return 0;
}

static inline int test_parseMethodCallOrAttr(void) {
    AriaLexer L = {0};
    ariaLexerInit(&L, "object.attr = 2 + 2 END");
    ariaTokenize(&L);

    ASTNode n = parseMethodCallOrAttr(&L);
    onetest_assert(n.type == AST_ASSIGN);

    onetest_assert(n.assign.ident->type == AST_IDENT);
    onetest_assert(strcmp(n.assign.ident->identifier, "attr") == 0);

    onetest_assert(n.assign.object_ident->type == AST_IDENT);
    onetest_assert(strcmp(n.assign.object_ident->identifier, "object") == 0);

    onetest_assert(n.assign.expr->type == AST_EXPR);
    onetest_assert(n.assign.expr->expr.op == TOK_PLUS);
    onetest_assert(n.assign.expr->expr.lhs->type == AST_NUM_LIT);
    onetest_assert(n.assign.expr->expr.rhs->type == AST_NUM_LIT);
    onetest_assert(n.assign.expr->expr.lhs->num_literal == 2);
    onetest_assert(n.assign.expr->expr.rhs->num_literal == 2);

    return 0;
}

static inline int test_parseType(void) {
    const char* type_str
        = "TYPE name\n"
          "VAR x BOOL VAR y NUM\n"
          "FUNC f1(arg BOOL) NIL ... END\n"
          "FUNC f2(arg1 NUM, arg2 NUM) NUM ... END\n"
          "END";

    AriaLexer L = {0};
    ariaLexerInit(&L, type_str);
    ariaTokenize(&L);

    ASTNode n = parseType(&L);
    onetest_assert(n.type == AST_TYPE);

    onetest_assert(strcmp(n.Type.name, "name") == 0);
    onetest_assert(n.Type.Vars.count == 2);
    onetest_assert(n.Type.Methods.count == 2);

    return 0;
}

static inline int test_parseVar(void) {
    AriaLexer L = {0};
    ariaLexerInit(&L, "VAR isLegal BOOL = TRUE END");
    ariaTokenize(&L);

    ASTNode n = parseVar(&L);
    onetest_assert(n.type == AST_VAR);

    onetest_assert(strcmp(n.var.name, "isLegal") == 0);
    onetest_assert(n.var.ret_type == TOK_BOOL);
    onetest_assert(n.var.value->bool_literal == true);

    return 0;
}

static inline int test_ariaCreateNode(void) { return 1; }

static inline int test_ariaParse(void) { return 1; }
