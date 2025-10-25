#ifndef ARIA_BYTECODE_H
#define ARIA_BYTECODE_H

#include "aria_buffer.h"
#include "aria_parser.h"

typedef enum {
    OP_STORE_CONST,
    OP_FUNC_CALL,
    OP_RET,
    OP_SUB,
    OP_ADD,
    OP_DIV,
    OP_MUL,
    OP_NOT,
    OP_NEQ,
    OP_ASSIGN,
    OP_EQ,
    OP_GT,
    OP_GE,
    OP_LT,
    OP_LE,
    OP_AND,
    OP_OR,
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

    // Aria_Buffer<str>
    Aria_Buffer* identifiers;
} Aria_Module;

Aria_Bytecode* appendPtr(Aria_Bytecode* inst, Aria_Bytecode* curr);
Aria_Bytecode* compileExpression(ASTNode* node, Aria_Bytecode* curr, Aria_Buffer* identifiers);
Aria_Chunk* compileFunc(ASTNode* node, Aria_Buffer* identifiers);
Aria_Module* ariaCompile(ASTNode* node);
const char* opcodeDisplay(Opcode op);
void printBytecode(const Aria_Module* mod);
void freeChunk(Aria_Chunk* chunk);

#endif // ARIA_BYTECODE_H
