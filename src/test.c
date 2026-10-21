#include "aria.c"

#define ONETEST_IMPLEMENTATION
#include "onetest.h"

int main() {
    onetest_test_t tests[] = {};

    return onetest_exec(tests, sizeof(tests) / sizeof(tests[0]));
}
