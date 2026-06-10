#ifndef ARIA_BYTECODE_H
#define ARIA_BYTECODE_H

#include "aria_parser.h"

typedef enum { OP_COUNT } Opcode;

// A single instruction
typedef struct _Bytecode {
    Opcode op;
    size_t operand_1;
    size_t operand_2;
    size_t operand_3;

    struct _Bytecode* next;
    struct _Bytecode* prev;
} Bytecode;

// A chunk is a named block (ex function, class)
// stores a linked list of instructions
typedef struct {
    char* name;
    Bytecode* buf;
} Aria_Chunk;

// A module is the contents of a given `.ari` file
typedef struct {
    char* name;
} Aria_Module;

Aria_Module ariaEmitBytecode(ASTNode module);

#endif  // ARIA_BYTECODE_H
