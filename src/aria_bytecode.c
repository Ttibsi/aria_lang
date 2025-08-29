#include "aria_bytecode.h"
#include <stdio.h>
#include <stdlib.h>

Bytecode* handleOperation(Bytecode* bc, Stack* stack, Expression* expr) {
    if(expr->op.lhs->type == Atom) {
        bc = handleAtom(bc, stack, expr->op.lhs);
    } else {
        bc = handleOperation(bc, stack, expr->op.lhs);
    }

    if (expr->op.rhs->type == Atom) {
        bc = handleAtom(bc, stack, expr->op.rhs);
    } else {
        bc = handleOperation(bc, stack, expr->op.rhs);
    }

    switch (expr->op.ch) {
        case '+':
            bc = nextInst(bc, INST_ADD);
            break;
        case '*':
            bc = nextInst(bc, INST_MUL);
    };

    return bc;
}

Bytecode* handleAtom(Bytecode* bc, Stack* stack, Expression* expr) {
    bc = nextInst(bc, INST_LOAD_CONST);
    stackPush(stack, expr->c);
    return bc;
}

Bytecode* nextInst(Bytecode* bc, Instruction inst) {
    bc->inst = inst;
    Bytecode* new = malloc(sizeof(Bytecode));
    new->inst = -1;
    bc->next = new;
    new->prev = bc;
    new->next = NULL;
    return bc->next;
}

Bytecode* bytecodeGeneration(Stack* stack, Expression expr) {
    Bytecode* root = malloc(sizeof(Bytecode));
    root->prev = NULL;

    if (expr.type == Atom) {
        handleAtom(root, stack, &expr);
    } else {
        handleOperation(root, stack, &expr);
    }

    return root;
}

void printBytecode(Bytecode* bc) {
    printf("\n=== BYTECODE ===\n");
    while (bc->next != NULL) {
        printf("Instruction: %d\n", bc->inst);
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
