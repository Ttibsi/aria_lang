#define ONETEST_IMPLEMENTATION
#include "onetest.h"

#include "test_lexer.h"

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
    };

    return onetest_exec(tests, sizeof(tests) / sizeof(tests[0]));
}
