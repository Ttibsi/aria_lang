#ifndef ARIA_BYTECODE_H
#define ARIA_BYTECODE_H

#include "aria_buffer.h"
#include "aria_parser.h"

typedef enum {
    OP_STORE_CONST,
    OP_NULL,
} Opcode;

typedef struct _Bytecode {
    Opcode op;
    int operand;
    struct _Bytecode* next;
    struct _Bytecode* prev;
} Aria_Bytecode;

typedef struct {
    char* name;
    Aria_Bytecode* buf;
} Aria_Chunk;

// Modules should only contain top-level constructs, such as functions,
// classes, import statements.
typedef struct {
    char* name;

    // Aria_Buffer<Aria_Chunk*>
    Aria_Buffer* buf;
} Aria_Module;

Aria_Chunk* compileFunc(ASTNode* node);
const char* opcodeDisplay(Opcode op);
Aria_Module* ariaCompile(ASTNode* node);
void printModule(const Aria_Module* mod);
void freeModule(Aria_Module* mod);

#endif // ARIA_BYTECODE_H
