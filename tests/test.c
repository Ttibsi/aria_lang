#define ONETEST_IMPLEMENTATION
#include "onetest.h"

#include "test_lexer.h"
#include "test_parser.h"
#include "test_bytecode.h"

int main(void) {
    onetest_test_t tests[] = {
        // test_lexer.h
		ONETEST_TEST(test_lexerInit),
		ONETEST_TEST(test_makeToken),
		ONETEST_TEST(test_peek),
		ONETEST_TEST(test_peekNext),
		ONETEST_TEST(test_advanceChar),
		ONETEST_TEST(test_skipWhitespace),
		ONETEST_TEST(test_scanEqualVariant),
		ONETEST_TEST(test_scanStringLiteral),
		ONETEST_TEST(test_scanNumber),
		ONETEST_TEST(test_scanIdentifier),
		ONETEST_TEST(test_scanToken),
		ONETEST_TEST(test_advance),
		ONETEST_TEST(test_check),
		ONETEST_TEST(test_match),
		ONETEST_TEST(test_getCurrTokenType),
		ONETEST_TEST(test_getTokenNumber),
		ONETEST_TEST(test_getTokenChar),

        // test_parser.h
        ONETEST_SKIP(test_parseConst),
        ONETEST_SKIP(test_parseFor),
        ONETEST_SKIP(test_parseIf),
        ONETEST_SKIP(test_parsePrint),
        ONETEST_TEST(test_parseReturn),
        ONETEST_SKIP(test_parseSwitch),
        ONETEST_SKIP(test_parseVar),
        ONETEST_SKIP(test_parseIdentifier),
        ONETEST_SKIP(test_parseExpression),
        ONETEST_SKIP(test_parseClass),
        ONETEST_SKIP(test_parseExport),
        ONETEST_SKIP(test_parseImport),
        ONETEST_TEST(test_parseBlock),
        ONETEST_TEST(test_parseFunc),
        ONETEST_TEST(test_ariaParse),
        ONETEST_TEST(test_createNode),

        // test_bytecode.h
        ONETEST_TEST(test_compileFunc),
        ONETEST_TEST(test_ariaCompile),
        ONETEST_TEST(test_opcodeDisplay),
    };

    return onetest_exec(tests, sizeof(tests) / sizeof(tests[0]));
}
