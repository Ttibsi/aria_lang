#ifndef ARIA_BYTECODE_H
#define ARIA_BYTECODE_H

#include "aria_parser.h"
#include "aria_stack.h"
#include "ht.h"

typedef enum {
    OP_ADD,
    OP_CALL,  // operand_1 is the heap index of the function name to call
    OP_DIV,
    OP_MUL,
    OP_RETURN,
    OP_STORE,
    OP_SUB,
} Opcode;

// A single instruction
typedef struct {
    Opcode op;
    size_t operand_1;
    size_t operand_2;
    size_t operand_3;
} Aria_Bytecode;

typedef struct {
    char** items;
    size_t count;
    size_t capacity;
} Heap;

// A chunk is a named block (ex function, class)
// stores a linked list of instructions
typedef struct {
    char* name;

    Aria_Bytecode* items;
    size_t count;
    size_t capacity;

    Heap heap;
} Aria_Chunk;

typedef Ht(const char*, Aria_Chunk) Chunk_map_t;

// A module is the contents of a given `.ari` file
// TODO: Square this up with AriaMod in aria.h (this should replace it)
typedef struct {
    char* name;
    Chunk_map_t chunks;
} Aria_Module;

void compileExpr(Aria_Chunk* chunk, ASTNode* node);
void compileStmt(Aria_Chunk* chunk, ASTNode* node);
Aria_Chunk compileFunc(ASTNode* node);
Aria_Module ariaEmitBytecode(ASTNode ast);

void printBytecode(Aria_Module* mod);
char* opcodeName(Opcode op);

#endif  // ARIA_BYTECODE_H
