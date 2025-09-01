#include "aria_executor.h"
#include "onetest.h"

static int test_ariaExecute(void) {
    Bytecode* bc = malloc(sizeof(Bytecode));

    bc->inst = INST_LOAD_CONST;
    bc->value = 5;
    bc->next = NULL;
    Stack* stack = ariaExecute(bc);
    onetest_assert(stackPeek(stack) == 5);
    freeStack(stack);

    bc->next = malloc(sizeof(Bytecode));
    bc->next->inst = INST_LOAD_CONST;
    bc->next->value = 3;
    bc->next->next = malloc(sizeof(Bytecode));
    bc->next->next->inst = INST_ADD;

    stack = ariaExecute(bc);
    onetest_assert(stackPeek(stack) == 8);

    freeStack(stack);
    freeBytecode(bc);
    return 0;
}
