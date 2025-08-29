#include "onetest.h"
#include "aria_stack.h"

static int test_isFull(void) {
    Stack* stack = createStack(1);
    stackPush(stack, 1);
    onetest_assert_eq(isFull(stack), 1);
    return 0;
}

static int test_isEmpty(void) {
    Stack* stack = createStack(1);
    onetest_assert_eq(isEmpty(stack), 1);
    stackPush(stack, 1);
    onetest_assert_ne(isEmpty(stack), 1);
    return 0;
}

static int test_stackPush(void) {
    Stack* stack = createStack(1);
    stackPush(stack, 1);
    onetest_assert_eq(*stack->array, 1);
    return 0;
}

static int test_stackPop(void) {
    Stack* stack = createStack(1);
    stackPush(stack, 1);
    onetest_assert_eq(stackPop(stack), 1);
    return 0;
}

static int test_stackPeek(void) {
    Stack* stack = createStack(1);
    stackPush(stack, 1);
    onetest_assert_eq(stackPeek(stack), 1);
    return 0;
}
