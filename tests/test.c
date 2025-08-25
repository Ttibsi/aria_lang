#define ONETEST_IMPLEMENTATION
#include "onetest.h"

#include "test_bytecode.h"
#include "test_lexer.h"
#include "test_stack.h"
#include "test_parser.h"

int main(void) {
    onetest_test_t tests[] = {
    };

    return onetest_exec(tests, sizeof(tests) / sizeof(tests[0]));
}
