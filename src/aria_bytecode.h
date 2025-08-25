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

Bytecode* handleOperation(Bytecode* bc, Stack* stack, Expression* expr);
Bytecode* handleAtom(Bytecode* bc, Stack* stack, Expression* expr);
Bytecode* nextInst(Bytecode* bc, Instruction inst);
Bytecode* bytecodeGeneration(Stack* stack, Expression expr);
void printBytecode(Bytecode* bc);
void freeBytecode(Bytecode* bc);

#endif // ARIA_BYTECODE_H
