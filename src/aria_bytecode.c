#include "aria_bytecode.h"
#include <stdio.h>
#include <stdlib.h>

Bytecode* handle_operation(Bytecode* bc, Stack* stack, Expression* expr) {
    if(expr->op.lhs->type == Atom) {
        bc = handle_atom(bc, stack, expr->op.lhs);
    } else {
        bc = handle_operation(bc, stack, expr->op.lhs);
    }

    if (expr->op.rhs->type == Atom) {
        bc = handle_atom(bc, stack, expr->op.rhs);
    } else {
        bc = handle_operation(bc, stack, expr->op.rhs);
    }

    switch (expr->op.ch) {
        case '+':
            bc = next_inst(bc, INST_ADD);
            break;
        case '*':
            bc = next_inst(bc, INST_MUL);
    };

    return bc;
}

Bytecode* handle_atom(Bytecode* bc, Stack* stack, Expression* expr) {
    bc = next_inst(bc, INST_LOAD_CONST);
    stackPush(stack, expr->c);
    return bc;
}

Bytecode* next_inst(Bytecode* bc, Instruction inst) {
    bc->inst = inst;
    Bytecode* new = malloc(sizeof(Bytecode));
    bc->next = new;
    new->prev = bc;
    new->next = NULL;
    return bc->next;
}

Bytecode* bytecode_generation(Stack* stack, Expression expr) {
    Bytecode* root = malloc(sizeof(Bytecode));
    root->prev = NULL;

    if (expr.type == Atom) {
        handle_atom(root, stack, &expr);
    } else {
        handle_operation(root, stack, &expr);
    }

    return root;
}

void print_bytecode(Bytecode* bc) {
    printf("\n=== BYTECODE ===\n");
    while (bc->next != NULL) {
        printf("Instruction: %d\n", bc->inst);
        bc = bc->next;
    }
}

void free_bytecode(Bytecode* bc) {
    if (bc == NULL) { return; }
    while (bc->prev != NULL) { bc = bc->prev; }

    while (bc != NULL) {
        Bytecode* next = bc->next;
        free(bc);
        bc = next;
    }
}
