#include "aria_bytecode.h"

#include <stddef.h>
#include <stdio.h>
#include <assert.h>

#define STACK_HEIGHT 64

// TODO: handle parameters -- first elems on the stack?
Aria_Chunk* compileFunc(ASTNode* node) {
    // need to duplicate for memory reasons

    Aria_Chunk* chunk = malloc(sizeof(Aria_Chunk));
    chunk->name = malloc(strlen(node->func.func_name) + 1);
    strcpy(chunk->name, node->func.func_name);
    chunk->buf = NULL;

    Aria_Buffer* body = node->func.body->block;
    Aria_Bytecode* current = chunk->buf;

    for (size_t i = 0; i < body->size; i++) {
        ASTNode* body_node = bufferGet(body, i);
        switch (body_node->type) {
            case AST_VALUE: {
                Aria_Bytecode* inst = malloc(sizeof(Aria_Bytecode));
                inst->op = OP_STORE_CONST;
                inst->operand = body_node->value;
                inst->next = NULL;
                inst->prev = NULL;

                if (current == NULL) {
                    current = inst;
                    chunk->buf = current;
                } else {
                    current->next = inst;
                    inst->prev = current;
                    current = inst;
                }

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
    mod->buf = bufferCreate(sizeof(Aria_Chunk), 64);

    assert(node->type == AST_MODULE);
    Aria_Buffer* module_buf = node->block;

    for (size_t i = 0; i < module_buf->size; i++) {
        ASTNode* child = (ASTNode*)bufferGet(module_buf, i);

        switch (child->type) {
            case AST_FUNC:
                Aria_Chunk* chunk = compileFunc(child);
                bufferInsert(mod->buf, chunk);

                // Contents is used in the buffer, so only free the chunk itself
                // allocated at the start of compileFunc
                free(chunk);
                break;
        }

    }

    return mod;
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
        Aria_Chunk* chunk = bufferGet(mod->buf, i);
        printf("  Chunk: %s\n", chunk->name);

        Aria_Bytecode* buf = chunk->buf;
        while (buf != NULL) {
            char display[4];
            if (buf->operand) {
                snprintf(display, 4, "%d", buf->operand);
            } else {
                display[0] = ' ';
                display[1] = '\0';
            }

            printf(
                "    %s: %s\n",
                opcodeDisplay(buf->op),
                display
            );


            buf = buf->next;
        }

    }
}

void printBytecode(const Aria_Module* mod) {
    printf("\n=== BYTECODE ===\n");
    printModule(mod);
}

void freeChunk(Aria_Chunk *chunk) {
    if (chunk == NULL) return;

    /* free the name string */
    free(chunk->name);
    chunk->name = NULL;

    /* free the linked list of bytecode instructions */
    Aria_Bytecode *cur = chunk->buf;
    while (cur != NULL) {
        Aria_Bytecode *next = cur->next;
        free(cur);
        cur = next;
    }
    chunk->buf = NULL;
}
