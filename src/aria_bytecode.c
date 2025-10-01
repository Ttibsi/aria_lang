#define ARIA_BUFFER_IMPL
#include "aria_bytecode.h"

#include <stddef.h>
#include <stdio.h>

// TODO: handle parameters -- first elems on the stack?
Aria_Chunk compileFunc(ASTNode node) {
    // need to duplicate for memory reasons
    char* chunk_name = strdup(node.func.func_name);

    Aria_Chunk chunk = {
        .name = chunk_name,
        .buf = bufferCreate(sizeof(Aria_Bytecode), 64)
    };

    Aria_Buffer body = node.func.body->block.buf;

    for (size_t i = 0; i < body.size; i++) {
        ASTNode* body_node = bufferGet(body, i);
        switch (body_node->type) {
            case AST_VALUE: {
                Aria_Bytecode inst = (Aria_Bytecode){OP_STORE_CONST, body_node->value};
                bufferInsert(&chunk.buf, (void*)&inst);
                break;
            }
            default: break;
        }
    }

    return chunk;
}

Aria_Module ariaCompile(ASTNode node) {
    Aria_Module mod = {"main", bufferCreate(sizeof(Aria_Chunk*), 64)};

    if (node.type == AST_FUNC) {
        Aria_Chunk chunk = compileFunc(node);
        bufferChunkInsert(&mod.buf, &chunk);
    }

    return mod;
}

// "Overload" for bufferInsert for Aria_Chunk -- so we can deep copy in
void bufferChunkInsert(Aria_Buffer* buf, Aria_Chunk* elem) {
    if (buf->size == buf->capacity) {
        buf->capacity *= 2;
        buf->items = realloc(buf->items, buf->capacity * buf->elem_size);
    }

    memcpy(buf->items + (buf->elem_size * buf->size), elem, buf->elem_size);
    buf->size++;

    Aria_Chunk* top = bufferPeek(*buf);
    memcpy(&top->buf, &elem->buf, sizeof(Aria_Buffer));
}

const char* opcodeDisplay(Opcode op) {
    switch (op) {
        case OP_STORE_CONST: return "OP_STORE_CONST";
    }

    return "";
}

void printModule(Aria_Module mod) {
    printf("Module: %s\n", mod.name);
    for (size_t i = 0; i < mod.buf.size; i++) {
        Aria_Chunk* chunk = bufferGet(mod.buf, i);
        printf("  Chunk: %s\n", chunk->name);

        for (size_t j = 0; j < chunk->buf.size; j++) {
            Aria_Bytecode* inst = bufferGet(chunk->buf, j);
            char display[4];
            if (inst->operand) {
                snprintf(display, 4, "%d", inst->operand);
            } else {
                display[0] = ' ';
                display[1] = '\0';
            }

            printf(
                "    %s: %s\n",
                opcodeDisplay(inst->op),
                display
            );

        }

    }
}
