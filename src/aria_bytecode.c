#define ARIA_BUFFER_IMPL
#include "aria_bytecode.h"

#include <stddef.h>
#include <stdio.h>
#include <assert.h>

// TODO: handle parameters -- first elems on the stack?
Aria_Chunk* compileFunc(ASTNode* node) {
    // need to duplicate for memory reasons

    Aria_Chunk* chunk = malloc(sizeof(Aria_Chunk));
    chunk->name = malloc(strlen(node->func.func_name));
    strcpy(chunk->name, node->func.func_name);
    chunk->buf = bufferCreate(sizeof(Aria_Bytecode), 64);

    Aria_Buffer* body = node->func.body->block.buf;

    for (size_t i = 0; i < body->size; i++) {
        ASTNode* body_node = bufferGet(body, i);
        switch (body_node->type) {
            case AST_VALUE: {
                Aria_Bytecode* inst = malloc(sizeof(Aria_Bytecode));
                inst->op = OP_STORE_CONST;
                inst->operand = body_node->value;
                bufferInsert(chunk->buf, inst);
                break;
            }
            default: break;
        }
    }

    return chunk;
}

Aria_Module* ariaCompile(ASTNode* node) {
    Aria_Module* mod = malloc(sizeof(Aria_Module));
    mod->name = "main";
    mod->buf = bufferCreate(sizeof(Aria_Chunk*), 64);

    assert(node->type == AST_MODULE);
    Aria_Buffer* module_buf = node->block.buf;

    for (size_t i = 0; i < module_buf->size; i++) {
        ASTNode* child = (ASTNode*)bufferGet(module_buf, i);

        switch (child->type) {
            case AST_FUNC:
                Aria_Chunk* chunk = compileFunc(child);
                bufferChunkInsert(mod->buf, chunk);
                break;
        }

    }

    return mod;
}

// "Overload" for bufferInsert for Aria_Chunk -- so we can deep copy in
void bufferChunkInsert(Aria_Buffer* buf, const Aria_Chunk* elem) {
    if (buf->size == buf->capacity) {
        buf->capacity *= 2;
        buf->items = realloc(buf->items, buf->capacity * buf->elem_size);
    }

    Aria_Chunk* dest_addr = (Aria_Chunk*)(buf->items + (buf->size * buf->elem_size));

    // Shallow copy
    *dest_addr =  *elem;
    // Deep copy buffers
    bufferCopy(dest_addr->buf, elem->buf);
    bufferCopy(dest_addr->stack, elem->stack);

    buf->size++;
}

Aria_Chunk* bufferChunkGet(const Aria_Buffer* buf, uint32_t idx) {
    if (idx >= buf->size) { return NULL; }

    Aria_Chunk* original = (Aria_Chunk*)(buf->items + (buf->elem_size * idx));

    // Allocate a new Aria_Chunk for deep copy
    Aria_Chunk* deep_copy = malloc(sizeof(Aria_Chunk));

    // Deep copy the name using strcpy
    if (original->name) {
        deep_copy->name = malloc(strlen(original->name) + 1);
        strcpy(deep_copy->name, original->name);
    } else {
        deep_copy->name = NULL;
    }

    deep_copy->buf = malloc(sizeof(Aria_Buffer));
    bufferCopy(deep_copy->buf, original->buf);

    deep_copy->stack = malloc(sizeof(Aria_Buffer));
    bufferCopy(deep_copy->stack, original->stack);

    return deep_copy;
}

const char* opcodeDisplay(Opcode op) {
    switch (op) {
        case OP_STORE_CONST: return "OP_STORE_CONST";
        default: return "UNKNOWN";
    }

    return "";
}

void printModule(const Aria_Module* mod) {
    printf("Module: %s\n", mod->name);
    for (size_t i = 0; i < mod->buf->size; i++) {
        Aria_Chunk* chunk = bufferChunkGet(mod->buf, i);
        printf("  Chunk: %s\n", chunk->name);

        for (size_t j = 0; j < chunk->buf->size; j++) {
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
