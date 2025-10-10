#include <stddef.h>
#include <stdio.h>

#define ARIA_STACK_IMPL
#include "aria_executor.h"

#define ARIA_HASH_IMPL
#include "aria_hash.h"

#define ARIA_BUFFER_IMPL
#include "aria_buffer.h"

// Map<str, Aria_Chunk>
static Map* dispatch_table = NULL;

void executeInstruction(Stack* global, Stack* local, Aria_Bytecode* inst) {
    switch (inst->op) {
        case OP_STORE_CONST:
            stackPush(local, inst->operand);
            break;
        case OP_NULL:
          break;
    }

    return;
}

int executeFunction(Stack* global_stack, Aria_Chunk* func) {
    Stack* local_stack = createStack(1024);

    Aria_Bytecode* playhead = func->buf;
    while (playhead != NULL) {
        executeInstruction(global_stack, local_stack, playhead);
        playhead = playhead->next;
    }

    return stackPeek(local_stack);
}

Stack* ariaExecute(Aria_Module* mod) {
    Stack* stack = createStack(1024);
    dispatch_table = mapCreate(sizeof(Aria_Chunk));

    for (size_t i = 0; i < mod->buf->size; i++) {
        Aria_Chunk* chunk = (Aria_Chunk*)bufferGet(mod->buf, i);
        int ret = mapInsert(dispatch_table, chunk->name, chunk);
        if (ret) {
            // do something here to show multiple definitions for the same function
        }
    }

    Aria_Chunk* main = mapFind(dispatch_table, "main");
    stackPush(stack, executeFunction(stack, main));

    freeModule(mod);
    mapFree(dispatch_table);
    return stack;
}
