#include <stddef.h>
#include <stdio.h>

#define ARIA_HASH_IMPL

#include "aria_buffer.h"
#include "aria_execute.h"
#include "aria_hash.h"


// Map<str, Aria_Chunk>
static Map* dispatch_table = NULL;

int executeInstruction(Stack* global, Stack* local, Aria_Bytecode* inst, Aria_Buffer* idents) {
    switch (inst->op) {
        case OP_STORE_CONST:
            stackPush(local, inst->operand);
            break;
        case OP_ADD: {
            int b = stackPop(local);
            int a = stackPop(local);
            stackPush(local, a + b);
        } break;
        case OP_MUL: {
            int b = stackPop(local);
            int a = stackPop(local);
            stackPush(local, a * b);
        } break;
        case OP_FUNC_CALL: {
            char* func_name = bufferGet(idents, inst->operand);
            Aria_Chunk* func = mapFind(dispatch_table, func_name);
            stackPush(local, executeFunction(global, func, idents));
        } break;
        case OP_RET: {
            return stackPeek(local);
        } break;
        case OP_NULL:
          break;
    }

    return 0;
}

int executeFunction(Stack* global_stack, Aria_Chunk* func, Aria_Buffer* idents) {
    Stack* local_stack = createStack(1024);

    Aria_Bytecode* playhead = func->buf;
    while (playhead != NULL) {
        executeInstruction(global_stack, local_stack, playhead, idents);
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
    stackPush(stack, executeFunction(stack, main, mod->identifiers));

    return stack;
}
