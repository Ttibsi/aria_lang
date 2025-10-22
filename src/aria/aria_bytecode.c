#include "aria_bytecode.h"

#include <stddef.h>
#include <stdio.h>
#include <assert.h>

#define STACK_HEIGHT 64

void appendPtr(Aria_Bytecode* inst, Aria_Bytecode* curr) {
    if (curr->op == OP_NULL) {
        free(curr);
        curr = inst;
        curr->prev = NULL;
        return;
    }

    curr->next = inst;
    inst->prev = curr;
    curr = inst;
}

Aria_Bytecode* compileSymbol(TokenType tok, Aria_Bytecode* curr) {
    Aria_Bytecode* inst = malloc(sizeof(Aria_Bytecode));
    inst->operand = 0;
    inst->next = NULL;
    inst->prev = NULL;

    switch (tok) {
        case TOK_MINUS: inst->op = OP_SUB; break;
        case TOK_PLUS: inst->op = OP_ADD; break;
        case TOK_SLASH: inst->op = OP_DIV; break;
        case TOK_STAR: inst->op = OP_MUL; break;
        case TOK_BANG: inst->op = OP_NOT; break;
        case TOK_BANG_EQUAL: inst->op = OP_NEQ; break;
        case TOK_EQUAL: inst->op = OP_ASSIGN; break;
        case TOK_EQUAL_EQUAL: inst->op = OP_EQ; break;
        case TOK_GREATER: inst->op = OP_GT; break;
        case TOK_GREATER_EQUAL: inst->op = OP_GE; break;
        case TOK_LESS: inst->op = OP_LT; break;
        case TOK_LESS_EQUAL: inst->op = OP_LE; break;
        case TOK_AND: inst->op = OP_AND; break;
        case TOK_OR: inst->op = OP_OR; break;
    }

    appendPtr(inst, curr);
    return inst;
}

Aria_Bytecode* compileExpression(ASTNode* node, Aria_Bytecode* curr, Aria_Buffer* identifiers) {
    switch (node->type) {
        case AST_VALUE: {
            Aria_Bytecode* inst = malloc(sizeof(Aria_Bytecode));
            inst->op = OP_STORE_CONST;
            inst->operand = node->value;
            inst->next = NULL;
            inst->prev = NULL;
            return inst;
        } break;

        case AST_FUNCCALL: {
            bufferInsert(identifiers, node->func_call.func_name);

            Aria_Bytecode* inst = malloc(sizeof(Aria_Bytecode));
            inst->op = OP_FUNC_CALL;
            inst->operand = identifiers->size;
            inst->next = NULL;
            inst->prev = NULL;
            return inst;
        } break;

        case AST_RETURN: {
            ASTNode* expr_node = node->ret.expr;
            Aria_Bytecode* expr = compileExpression(expr_node, curr, identifiers);
            appendPtr(expr, curr);

            Aria_Bytecode* inst = malloc(sizeof(Aria_Bytecode));
            inst->op = OP_RET;
            inst->operand = -1;
            inst->next = NULL;
            inst->prev = expr;
            appendPtr(inst, curr);

            return inst;
        } break;

        case AST_EXPR: {
            Aria_Bytecode* expr = compileExpression(node->expr.lhs, curr, identifiers);
            appendPtr(expr, curr);
            expr = compileExpression(node->expr.rhs, curr, identifiers);
            appendPtr(expr, curr);

            return compileSymbol(node->expr.op, curr);
        }

        default: break;
    }

    unreachable();
}

// TODO: handle parameters -- first elems on the stack?
Aria_Chunk* compileFunc(ASTNode* node, Aria_Buffer* identifiers) {
    // need to duplicate for memory reasons

    Aria_Chunk* chunk = malloc(sizeof(Aria_Chunk));
    chunk->name = malloc(strlen(node->func.func_name) + 1);
    strcpy(chunk->name, node->func.func_name);

    chunk->buf = malloc(sizeof(Aria_Bytecode));
    chunk->buf->op = OP_NULL;
    chunk->buf->operand = -1;
    chunk->buf->next = NULL;
    chunk->buf->prev = NULL;

    Aria_Buffer* body = node->func.body->block;
    Aria_Bytecode* curr = chunk->buf;

    for (size_t i = 0; i < body->size; i++) {
        ASTNode* body_node = bufferGet(body, i);
        Aria_Bytecode* inst = compileExpression(body_node, curr, identifiers);

        appendPtr(inst, curr);
        if (chunk->buf == NULL) { chunk->buf = curr; }
    }

    return chunk;
}

Aria_Module* ariaCompile(ASTNode* node) {
    Aria_Module* mod = malloc(sizeof(Aria_Module));
    mod->name = "main";
    mod->buf = bufferCreate(sizeof(Aria_Chunk), 64);
    mod->identifiers = bufferCreate(sizeof(char*), 32);

    assert(node->type == AST_MODULE);
    Aria_Buffer* module_buf = node->block;

    for (size_t i = 0; i < module_buf->size; i++) {
        ASTNode* child = (ASTNode*)bufferGet(module_buf, i);

        switch (child->type) {
            case AST_FUNC:
                Aria_Chunk* chunk = compileFunc(child, mod->identifiers);
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
        case OP_STORE_CONST:  return "OP_STORE_CONST";
        case OP_FUNC_CALL:    return "OP_FUNC_CALL";
        case OP_RET:          return "OP_RET";
        case OP_MUL:          return "OP_MUL";
        case OP_ADD:          return "OP_ADD";
        default:              return "UNKNOWN";
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
            if (buf->operand != -1) {
                snprintf(display, 4, "%d", buf->operand);
            } else {
                display[0] = ' ';
                display[1] = '\0';
            }

            printf(
                "%*s%s: %s\n",
                4, "",
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
