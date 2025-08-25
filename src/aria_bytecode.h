#ifndef ARIA_BYTECODE_H
#define ARIA_BYTECODE_H

#include "aria_parser.h"
#include "aria_stack.h"

typedef enum {
    INST_LOAD_CONST,
    INST_ADD,
    INST_MUL,
} Instruction;

typedef struct _Bytecode {
    Instruction inst;
    struct _Bytecode* next;
    struct _Bytecode* prev;
} Bytecode;

Bytecode* handle_operation(Bytecode* bc, Stack* stack, Expression* expr);
Bytecode* handle_atom(Bytecode* bc, Stack* stack, Expression* expr);
Bytecode* next_inst(Bytecode* bc, Instruction inst);
Bytecode* bytecode_generation(Stack* stack, Expression expr);
void print_bytecode(Bytecode* bc);
void free_bytecode(Bytecode* bc);

#endif // ARIA_BYTECODE_H
