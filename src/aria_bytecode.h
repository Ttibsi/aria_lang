#ifndef ARIA_BYTECODE_H
#define ARIA_BYTECODE_H

#include "aria_parser.h"
#include "aria_stack.h"

typedef enum {
    Inst_ADD,
} Instruction;

typedef struct _Bytecode {
    Instruction inst;
    struct _Bytecode* next;
    struct _Bytecode* prev;
} Bytecode;

void handle_operation(Bytecode* bc, Stack* stack, Expression* expr);
void handle_atom(Bytecode* bc, Stack* stack, Expression* expr);
void next_inst(Bytecode* bc, Instruction inst);
Bytecode* bytecode_generation(Stack* stack, Expression expr);

#endif // ARIA_BYTECODE_H
