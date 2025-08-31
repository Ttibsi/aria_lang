#include "aria_bytecode.h"
#include <stdio.h>
#include <stdlib.h>

Bytecode* handleOperation(Bytecode* bc, Expression* expr) {
    if(expr->op.lhs->type == Atom) {
        bc = handleAtom(bc, expr->op.lhs);
    } else {
        bc = handleOperation(bc, expr->op.lhs);
    }

    if (expr->op.rhs->type == Atom) {
        bc = handleAtom(bc, expr->op.rhs);
    } else {
        bc = handleOperation(bc, expr->op.rhs);
    }

    switch (expr->op.ch) {
        case '+':
            bc = nextInst(bc, INST_ADD, 0);
            break;
        case '*':
            bc = nextInst(bc, INST_MUL, 0);
    };

    return bc;
}

Bytecode* handleAtom(Bytecode* bc, Expression* expr) {
    bc = nextInst(bc, INST_LOAD_CONST, expr->c);
    return bc;
}

Bytecode* nextInst(Bytecode* bc, Instruction inst, int value) {
    bc->inst = inst;
    bc->value = value;
    Bytecode* new = malloc(sizeof(Bytecode));
    new->inst = INST_NULL;
    bc->next = new;
    new->prev = bc;
    new->next = NULL;
    return bc->next;
}

Bytecode* bytecodeGeneration(Expression expr) {
    Bytecode* root = malloc(sizeof(Bytecode));
    root->prev = NULL;

    if (expr.type == Atom) {
        handleAtom(root, &expr);
    } else {
        handleOperation(root, &expr);
    }

    return root;
}

void printBytecode(Bytecode* bc) {
    printf("\n=== BYTECODE ===\n");
    while (bc->next != NULL) {
        printf("Instruction: %d Value: %d\n", bc->inst, bc->value);
        bc = bc->next;
    }
}

void freeBytecode(Bytecode* bc) {
    if (bc == NULL) { return; }
    while (bc->prev != NULL) { bc = bc->prev; }

    while (bc != NULL) {
        Bytecode* next = bc->next;
        free(bc);
        bc = next;
    }
}
