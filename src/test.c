#include "aria.c"

#define ONETEST_IMPLEMENTATION
#include "onetest.h"

int test_hash(void) { return 1; }
int test_make_str_from_cstr(void) { return 1; }
int test_make_str_from_offset(void) { return 1; }
int test_str_cmp(void) { return 1; }
int test_table_find(void) { return 1; }
int test_table_get(void) { return 1; }
int test_table_set(void) { return 1; }
int test_aria_vm_cleanup(void) { return 1; }

int test_makeToken(void) { return 1; }
int test_peek(void) { return 1; }
int test_peekNext(void) { return 1; }
int test_advanceChar(void) { return 1; }
int test_skipWhitespace(void) { return 1; }
int test_advanceComment(void) { return 1; }
int test_scanEqualVariant(void) { return 1; }
int test_scanStringLiteral(void) { return 1; }
int test_scanNumber(void) { return 1; }
int test_scanIdentifier(void) { return 1; }
int test_scanToken(void) { return 1; }

int test_check(void) { return 1; }
int test_checkType(void) { return 1; }
int test_match(void) { return 1; }
int test_advanceTok(void) { return 1; }
int test_setError(void) { return 1; }
int test_emitByte(void) { return 1; }
int test_emitBytes(void) { return 1; }
int test_emitJump(void) { return 1; }
int test_patchJump(void) { return 1; }
int test_emitLoop(void) { return 1; }
int test_identifierConstant(void) { return 1; }
int test_parseImport(void) { return 1; }
int test_isStatementBoundary(void) { return 1; }
int test_syncToStatementBoundary(void) { return 1; }
int test_isExprStop(void) { return 1; }
int test_getInfixPrecedence(void) { return 1; }
int test_emitBinaryOp(void) { return 1; }
int test_parsePrefix(void) { return 1; }
int test_parsePrecedence(void) { return 1; }
int test_parseExpressionUntil(void) { return 1; }
int test_parseVariable(void) { return 1; }
int test_parseIf(void) { return 1; }
int test_parseFor(void) { return 1; }
int test_parseForEach(void) { return 1; }
int test_parseReturn(void) { return 1; }
int test_parseIdentifier(void) { return 1; }
int test_parseStatement(void) { return 1; }
int test_parseBlock(void) { return 1; }
int test_parseFunc(void) { return 1; }

int test_op_name(void) { return 1; }
int test_dump_function_ops(void) { return 1; }
int test_dump_all_functions(void) { return 1; }
int test_parse(void) { return 1; }

int test_pop(void) { return 1; }
int test_execute(void) { return 1; }
int test_aria_load_file(void) { return 1; }
int test_aria_call_func(void) { return 1; }

int main(void) {
    onetest_test_t tests[] = {
        ONETEST_TEST(test_hash),
        ONETEST_TEST(test_make_str_from_cstr),
        ONETEST_TEST(test_make_str_from_offset),
        ONETEST_TEST(test_str_cmp),
        ONETEST_TEST(test_table_find),
        ONETEST_TEST(test_table_get),
        ONETEST_TEST(test_table_set),
        ONETEST_TEST(test_aria_vm_cleanup),
        ONETEST_TEST(test_makeToken),
        ONETEST_TEST(test_peek),
        ONETEST_TEST(test_peekNext),
        ONETEST_TEST(test_advanceChar),
        ONETEST_TEST(test_skipWhitespace),
        ONETEST_TEST(test_advanceComment),
        ONETEST_TEST(test_scanEqualVariant),
        ONETEST_TEST(test_scanStringLiteral),
        ONETEST_TEST(test_scanNumber),
        ONETEST_TEST(test_scanIdentifier),
        ONETEST_TEST(test_scanToken),
        ONETEST_TEST(test_check),
        ONETEST_TEST(test_checkType),
        ONETEST_TEST(test_match),
        ONETEST_TEST(test_advanceTok),
        ONETEST_TEST(test_setError),
        ONETEST_TEST(test_emitByte),
        ONETEST_TEST(test_emitBytes),
        ONETEST_TEST(test_emitJump),
        ONETEST_TEST(test_patchJump),
        ONETEST_TEST(test_emitLoop),
        ONETEST_TEST(test_identifierConstant),
        ONETEST_TEST(test_parseImport),
        ONETEST_TEST(test_isStatementBoundary),
        ONETEST_TEST(test_syncToStatementBoundary),
        ONETEST_TEST(test_isExprStop),
        ONETEST_TEST(test_getInfixPrecedence),
        ONETEST_TEST(test_emitBinaryOp),
        ONETEST_TEST(test_parsePrefix),
        ONETEST_TEST(test_parsePrecedence),
        ONETEST_TEST(test_parseExpressionUntil),
        ONETEST_TEST(test_parseVariable),
        ONETEST_TEST(test_parseIf),
        ONETEST_TEST(test_parseFor),
        ONETEST_TEST(test_parseForEach),
        ONETEST_TEST(test_parseReturn),
        ONETEST_TEST(test_parseIdentifier),
        ONETEST_TEST(test_parseStatement),
        ONETEST_TEST(test_parseBlock),
        ONETEST_TEST(test_parseFunc),
        ONETEST_TEST(test_op_name),
        ONETEST_TEST(test_dump_function_ops),
        ONETEST_TEST(test_dump_all_functions),
        ONETEST_TEST(test_parse),
        ONETEST_TEST(test_pop),
        ONETEST_TEST(test_execute),
        ONETEST_TEST(test_aria_load_file),
        ONETEST_TEST(test_aria_call_func),
    };

    return onetest_exec(tests, sizeof(tests) / sizeof(tests[0]));
}
