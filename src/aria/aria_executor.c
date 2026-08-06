#include "aria_executor.h"

#include <assert.h>

#include "ht.h"
#include "nob.h"

OptionalInt executeInst(Aria_Bytecode* bc, Stack* stack, Aria_Chunk* c, Chunk_map_t* chunks) {
    int a = 0;
    int b = 0;

    switch (bc->op) {
        case OP_ADD: {
            assert(bc->operand_count == 1);
            assert(stack->capacity >= 2);

            b = stackPop(stack);
            a = stackPop(stack);
            stackPush(stack, a + b);
        } break;

        case OP_CALL: {
            assert(bc->operand_count == 1);
            const char* callee = c->mod->heap.items[bc->operand_1];
            OptionalInt funcCall = executeChunk(ht_find(chunks, callee), chunks);
            if (funcCall.has_value) { stackPush(stack, funcCall.value); }
        } break;

        case OP_DIV: {
            assert(bc->operand_count == 1);
            assert(stack->capacity >= 2);
            b = stackPop(stack);
            a = stackPop(stack);
            stackPush(stack, a / b);
        } break;

        case OP_MUL: {
            assert(bc->operand_count == 1);
            assert(stack->capacity >= 2);
            b = stackPop(stack);
            a = stackPop(stack);
            stackPush(stack, a * b);
        } break;

        case OP_RETURN: {
            assert(bc->operand_count == 1);
            return (OptionalInt){true, stackPop(stack)};
        } break;

        case OP_STORE: {
            assert(bc->operand_count == 2);
            stack->items[bc->operand_2] = bc->operand_1;
        } break;

        case OP_LOAD: {
            assert(bc->operand_count == 2);
            const char* name = c->mod->heap.items[bc->operand_1];
        } break;

        case OP_SUB: {
            assert(bc->operand_count == 1);
            assert(stack->capacity >= 2);
            b = stackPop(stack);
            a = stackPop(stack);
            stackPush(stack, a - b);
        } break;
    }

    return (OptionalInt){false, 0};
}

[[nodiscard]] OptionalInt executeChunk(Aria_Chunk* c, Chunk_map_t* map) {
    Stack* stack = createStack(NOB_DA_INIT_CAP);
    OptionalInt ret;

    nob_da_foreach(Aria_Bytecode, bc, c) {
        ret = executeInst(bc, stack, c, map);
        if (ret.has_value) { break; }
    }

    return ret;
}

[[nodiscard]] int ariaExecute(Aria_Module mod) {
    Aria_Chunk* main = ht_find(&mod.chunks, "main");
    OptionalInt ret = executeChunk(main, &mod.chunks);

    if (ret.has_value) { return ret.value; }
    return 0;
}
