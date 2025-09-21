#define ONETEST_IMPLEMENTATION
#include "onetest.h"

#include "test_buffer.h"
// #include "test_bytecode.h"
#include "test_lexer.h"
// #include "test_stack.h"
// #include "test_executor.h"
// #include "test_interpreter.h"
#include "test_hash.h"

int main(void) {
    onetest_test_t tests[] = {
        // test_buffer.h
        ONETEST_TEST(test_bufferCreate),
        ONETEST_TEST(test_bufferInsert),
        ONETEST_TEST(test_bufferGet),
        // test_bytecode.h
        // ONETEST_TEST(test_handleOperation),
        // ONETEST_TEST(test_handleAtom),
        // ONETEST_TEST(test_nextInst),
        // ONETEST_TEST(test_bytecodeGeneration),
        // test_lexer.h
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
        // ONETEST_TEST(test_isFull),
        // ONETEST_TEST(test_isEmpty),
        // ONETEST_TEST(test_stackPush),
        // ONETEST_TEST(test_stackPop),
        // ONETEST_TEST(test_stackPeek),
        // ONETEST_TEST(test_ariaExecute),
        // test_interpreter.h
        // ONETEST_TEST(test_add),
        // ONETEST_TEST(test_subtract),
        // ONETEST_TEST(test_multiplication),
        // ONETEST_TEST(test_division),
        // ONETEST_TEST(test_brackets),
        // ONETEST_TEST(test_printing),
        // test_hash.h
        ONETEST_TEST(test_mapCreate),
        ONETEST_TEST(test_hash),
        ONETEST_TEST(test_mapInsert),
        ONETEST_TEST(test_mapRemove),
        ONETEST_TEST(test_mapExists),
        ONETEST_TEST(test_mapFind),
    };

    return onetest_exec(tests, sizeof(tests) / sizeof(tests[0]));
}
