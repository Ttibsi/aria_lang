#include "onetest.h"
#include "aria.h"

static int test_add(void) {
    int result = ariaInterpret("5 + 3");
    onetest_assert(result == 8);
    return 0;
}

static int test_subtract(void) {
    int result = ariaInterpret("5 - 3");
    onetest_assert(result == 2);
    return 0;
}

static int test_multiplication(void) {
    int result = ariaInterpret("5 * 2");
    onetest_assert(result == 10);
    return 0;
}

static int test_division(void) {
    int result = ariaInterpret("10/2");
    onetest_assert(result == 5);
    return 0;
}

static int test_brackets(void) {
    // Intentionally mixing up spaces between numbers and not
    int result = ariaInterpret("35 + ( 17*2)");
    onetest_assert(result == 69);
    return 0;
}
