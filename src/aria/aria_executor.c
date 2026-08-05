#include "aria_executor.h"

#include <assert.h>

#include "ht.h"
#include "nob.h"

OptionalInt executeInst(Aria_Bytecode* bc, Stack* stack, Aria_Chunk* c, Chunk_map_t* chunks) {
    int a = 0;
    int b = 0;

    switch (bc->op) {
        case OP_ADD: {
            assert(stack->capacity >= 2);
            b = stackPop(stack);
            a = stackPop(stack);
            stackPush(stack, a + b);
        } break;

        case OP_CALL: {
            const char* callee = c->mod->heap.items[bc->operand_1];
            OptionalInt funcCall = executeChunk(ht_find(chunks, callee), chunks);
            if (funcCall.has_value) { stackPush(stack, funcCall.value); }
        } break;

        case OP_DIV: {
            assert(stack->capacity >= 2);
            b = stackPop(stack);
            a = stackPop(stack);
            stackPush(stack, a / b);
        } break;

        case OP_MUL: {
            assert(stack->capacity >= 2);
            b = stackPop(stack);
            a = stackPop(stack);
            stackPush(stack, a * b);
        } break;

        case OP_RETURN: {
            return (OptionalInt){true, stackPop(stack)};
        } break;

        case OP_STORE: {
            assert(bc->operand_count == 2);
            stackPush(stack, bc->operand_1);
        } break;

        case OP_LOAD: {
            const char* name = c->heap.items[bc->operand_1];
        } break;

        case OP_SUB: {
            assert(stack->capacity >= 2);
            b = stackPop(stack);
            a = stackPop(stack);
            stackPush(stack, a - b);
        } break;
    }

    return (OptionalInt){false, 0};
}

[[nodiscard]] OptionalInt executeChunk(Aria_Chunk* c, Chunk_map_t* map) {
    Stack* stack = createStack(1024);
    OptionalInt ret;

    nob_da_foreach(Aria_Bytecode, bc, c) {
        ret = executeInst(bc, stack, c, map);
        if (ret.has_value) { break; }
    }

    freeStack(stack);
    return ret;
}

[[nodiscard]] int ariaExecute(Aria_Module mod) {
    Aria_Chunk* main = ht_find(&mod.chunks, "main");
    OptionalInt ret = executeChunk(main, &mod.chunks);

    if (ret.has_value) { return ret.value; }
    return 0;
}
