#define ONETEST_IMPLEMENTATION
#include "lexer_test.h"
#include "onetest.h"
#include "parser_test.h"

int main(void) {
    onetest_test_t tests[] = {
        // lexer_test.h
        ONETEST_TEST(test_ariaLexerInit),
        ONETEST_TEST(test_makeToken),
        ONETEST_TEST(test_peek),
        ONETEST_TEST(test_peekNext),
        ONETEST_TEST(test_advanceChar),
        ONETEST_TEST(test_skipWhitespace),
        ONETEST_TEST(test_scanEqualVariant),
        ONETEST_TEST(test_scanStringLiteral),
        ONETEST_TEST(test_scanNumber),
        ONETEST_TEST(test_advanceComment),
        ONETEST_TEST(test_scanIdentifier),
        ONETEST_TEST(test_scanToken),
        ONETEST_TEST(test_advance),
        ONETEST_TEST(test_check),
        ONETEST_TEST(test_match),
        ONETEST_TEST(test_checkType),
        ONETEST_TEST(test_isKeyword),
        ONETEST_TEST(test_getCurrTokenType),
        ONETEST_TEST(test_getTokenNumber),
        ONETEST_TEST(test_getTokenChar),
        ONETEST_TEST(test_getTokenString),

        // parser_test.h
        ONETEST_TEST(test_parseArg),
        ONETEST_TEST(test_parseAssignment),
        ONETEST_TEST(test_parseBlock),
        ONETEST_TEST(test_parseFunc),
        ONETEST_TEST(test_parseFuncCall),
        ONETEST_TEST(test_parseExpression),
        ONETEST_TEST(test_parseFor),
        ONETEST_TEST(test_parseForEach),
        ONETEST_TEST(test_parseIdentifier),
        ONETEST_TEST(test_parseIf),
        ONETEST_TEST(test_parseImport),
        ONETEST_TEST(test_parseReturn),
        ONETEST_TEST(test_parseMethodCall),
        ONETEST_TEST(test_parseMethodCallOrAttr),
        ONETEST_TEST(test_parseType),
        ONETEST_TEST(test_parseVar),
        ONETEST_TEST(test_ariaCreateNode),
        ONETEST_TEST(test_ariaParse),
    };

    const size_t test_count = sizeof(tests) / sizeof(tests[0]);
    return onetest_exec(tests, test_count);
}
