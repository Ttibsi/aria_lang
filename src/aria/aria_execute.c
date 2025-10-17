#include <stddef.h>
#include <stdio.h>

#define ARIA_HASH_IMPL

#include "aria_buffer.h"
#include "aria_execute.h"
#include "aria_hash.h"


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

    int top = stackPeek(local_stack);
    freeStack(local_stack);
    return top;
}

Stack* ariaExecute(Aria_Module* mod) {
    Stack* stack = createStack(1024);
    dispatch_table = mapCreate(sizeof(Aria_Chunk));

    for (size_t i = 0; i < mod->buf->size; i++) {
        Aria_Chunk* chunk = (Aria_Chunk*)bufferGet(mod->buf, i);
        // Most interpreted languages don't warn on redeclaration of a function (python, lua, js)
        // so we don't need to check/do anything if a function is redeclared, it just gets
        // written over
        mapInsert(dispatch_table, chunk->name, chunk);

    }

    Aria_Chunk* main = mapFind(dispatch_table, "main");
    stackPush(stack, executeFunction(stack, main));

    return stack;
}
