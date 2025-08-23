#include "aria_bytecode.h"
#include <stdlib.h>

void handle_operation(Bytecode* bc, Stack* stack, Expression* expr) {
    if(expr->op.lhs->type == Atom) {
        handle_atom(bc, stack, expr->op.lhs);
    } else {
        handle_operation(bc, stack, expr->op.lhs);
    }

    if (expr->op.rhs->type == Atom) {
        handle_atom(bc, stack, expr->op.rhs);
    } else {
        handle_operation(bc, stack, expr->op.rhs);
    }

    switch (expr->op.ch) {
        case '+':
            next_inst(bc, Inst_ADD);
            break;
    };
}

void handle_atom(Bytecode* bc, Stack* stack, Expression* expr) {
}

void next_inst(Bytecode* bc, Instruction inst) {
    Bytecode* new = malloc(sizeof(Bytecode));
    bc->next = new;
    new->prev = bc;
}

Bytecode* bytecode_generation(Stack* stack, Expression expr) {
    Bytecode* root = malloc(sizeof(Bytecode));

    if (expr.type == Atom) {
        handle_atom(root, stack, &expr);
    } else {
        handle_operation(root, stack, &expr);
    }

    return root;
}
