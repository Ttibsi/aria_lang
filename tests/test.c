#define ONETEST_IMPLEMENTATION
#include "onetest.h"

#include "test_bytecode.h"
#include "test_lexer.h"
#include "test_stack.h"
#include "test_parser.h"

int main(void) {
    onetest_test_t tests[] = {
        ONETEST_TEST(test_handleOperation),
        ONETEST_TEST(test_handleAtom),
        ONETEST_TEST(test_nextInst),
        ONETEST_TEST(test_bytecodeGeneration),
        ONETEST_TEST(test_peek),
        ONETEST_TEST(test_peekNext),
        ONETEST_TEST(test_advanceChar),
        ONETEST_TEST(test_skipWhitespace),
        ONETEST_TEST(test_makeToken),
        ONETEST_TEST(test_scanEqualVariant),
        ONETEST_TEST(test_scanStringLiteral),
        ONETEST_TEST(test_scanNumber),
        ONETEST_TEST(test_scanIdentifier),
        ONETEST_TEST(test_scanToken),
        ONETEST_TEST(test_advance),
        ONETEST_TEST(test_check),
        ONETEST_TEST(test_match),
        ONETEST_TEST(test_getTokenNumber),
        ONETEST_TEST(test_getTokenChar),
        ONETEST_TEST(test_infixBindingPower),
        ONETEST_TEST(test_advanceState),
        ONETEST_TEST(test_parseExpression),
        ONETEST_TEST(test_isFull),
        ONETEST_TEST(test_isEmpty),
        ONETEST_TEST(test_stackPush),
        ONETEST_TEST(test_stackPop),
        ONETEST_TEST(test_stackPeek),
    };

    return onetest_exec(tests, sizeof(tests) / sizeof(tests[0]));
}
