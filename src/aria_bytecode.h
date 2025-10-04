#ifndef ARIA_BYTECODE_H
#define ARIA_BYTECODE_H

#include "aria_buffer.h"
#include "aria_parser.h"

typedef enum {
    FOO,
    OP_STORE_CONST,
} Opcode;

typedef struct {
    Opcode op;
    int operand;
} Aria_Bytecode;

typedef struct {
    char* name;
    // Aria_Buffer<Aria_Bytecode>
    Aria_Buffer* buf;

    // Aria_Buffer<int>
    Aria_Buffer* stack;
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
void bufferChunkInsert(Aria_Buffer* buf, const Aria_Chunk* elem);
void printModule(const Aria_Module* mod);

#endif // ARIA_BYTECODE_H
