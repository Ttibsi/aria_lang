#include "onetest.h"
#include "aria_stack.h"

static int test_isFull(void) {
    Stack* stack = createStack(1);
    stackPush(stack, 1);
    onetest_assert(isFull(stack) == 1);
    return 0;
}

static int test_isEmpty(void) {
    Stack* stack = createStack(1);
    onetest_assert(isEmpty(stack) == 1);
    stackPush(stack, 1);
    onetest_assert(isEmpty(stack) != 1);
    return 0;
}

static int test_stackPush(void) {
    Stack* stack = createStack(1);
    stackPush(stack, 1);
    onetest_assert(*stack->array == 1);
    return 0;
}

static int test_stackPop(void) {
    Stack* stack = createStack(1);
    stackPush(stack, 1);
    onetest_assert(stackPop(stack) == 1);
    return 0;
}

static int test_stackPeek(void) {
    Stack* stack = createStack(1);
    stackPush(stack, 1);
    onetest_assert(stackPeek(stack) == 1);
    return 0;
}
