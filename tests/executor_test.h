#include "aria/aria_executor.h"
#include "onetest.h"

static inline int test_executeInst(void) {
    Stack* stack = createStack(4);

    Aria_Bytecode bc_1 = {.op = OP_STORE, .operand_1 = 6};
    OptionalInt ret = executeInst(&bc_1, stack, NULL, NULL);
    if (ret.has_value) { return 1; }

    Aria_Bytecode bc_2 = {.op = OP_STORE, .operand_1 = 6};
    ret = executeInst(&bc_2, stack, NULL, NULL);
    if (ret.has_value) { return 2; }

    Aria_Bytecode bc_3 = {.op = OP_ADD};
    ret = executeInst(&bc_3, stack, NULL, NULL);
    if (ret.has_value) { return 3; }

    int x = stackPop(stack);
    if (x != 12) { return 4; }

    return 0;
}

static inline int test_executeChunk(void) {
    Aria_Chunk chunk = {0};

    Aria_Bytecode bc_1 = {.op = OP_STORE, .operand_1 = 6};
    Aria_Bytecode bc_2 = {.op = OP_STORE, .operand_1 = 6};
    Aria_Bytecode bc_3 = {.op = OP_ADD};
    Aria_Bytecode bc_4 = {.op = OP_RETURN};
    nob_da_append(&chunk, bc_1);
    nob_da_append(&chunk, bc_2);
    nob_da_append(&chunk, bc_3);
    nob_da_append(&chunk, bc_4);

    OptionalInt ret = executeChunk(&chunk, NULL);
    if (!ret.has_value) { return 1; }
    if (ret.value != 12) { return 2; }

    return 0;
}
