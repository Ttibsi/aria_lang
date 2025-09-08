#ifndef ARIA_BYTECODE_H
#define ARIA_BYTECODE_H

#include "aria_parser.h"

typedef enum {
    INST_LOAD_CONST,
    INST_ADD,
    INST_SUB,
    INST_MUL,
    INST_DIV,
    INST_PRINT,

    INST_NULL,
} Instruction;

typedef struct _Bytecode {
    Instruction inst;
    int value; // Can be expanded to multiple opcodes in the future. linked list?
    struct _Bytecode* next;
    struct _Bytecode* prev;
} Bytecode;

Bytecode* handleOperation(Bytecode* bc, Expression* expr);
Bytecode* handleAtom(Bytecode* bc, Expression* expr);
Bytecode* nextInst(Bytecode* bc, Instruction inst, int value);
Bytecode* bytecodeGeneration(Expression expr);
void printBytecode(Bytecode* bc);
void freeBytecode(Bytecode* bc);

#endif // ARIA_BYTECODE_H
