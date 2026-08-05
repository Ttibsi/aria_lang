#ifndef ARIA_BYTECODE_H
#define ARIA_BYTECODE_H

#include "aria_parser.h"
#include "aria_stack.h"
#include "ht.h"

#define STACK_SIZE 1024 * 2

typedef enum {
    OP_ADD,
    OP_CALL,  // operand_1 is the heap index of the function name to call
    OP_DIV,
    OP_MUL,
    OP_RETURN,
    OP_STORE,  // operand_1 = value to store, operand_2 = offset of space in stack
    OP_LOAD,   // Load the variable with the name at location operand_1
    OP_SUB,
    OP_JUMP_COND,  // Jump ahead operand_1 instructions if stack top is 1
    OP_JUMP,
    OP_LT,
    OP_GT,
    OP_LE,
    OP_GE,
    OP_EQ,  // ==
    OP_NE,
    OP_STACK_REWIND,
    OP_STACK_SAVE
} Opcode;

// A single instruction
typedef struct {
    Opcode op;
    size_t operand_1;
    size_t operand_2;
    size_t operand_3;

    size_t operand_count;  // How many operands are populated?
} Aria_Bytecode;

typedef struct {
    char** items;
    size_t count;
    size_t capacity;
} Heap;

// Keys are variable/function names
// values are the stack offset of those values
typedef Ht(const char*, size_t) Symbol_table_t;
typedef struct Aria_Module Aria_Module;

// A chunk is a named block (ex function, class)
// stores a linked list of instructions
typedef struct {
    char* name;

    Aria_Bytecode* items;
    size_t count;
    size_t capacity;

    size_t stackStart;
    Symbol_table_t symtab;
    Aria_Module* mod;
} Aria_Chunk;

typedef Ht(const char*, Aria_Chunk) Chunk_map_t;

// A module is the contents of a given `.ari` file
// TODO: Square this up with AriaMod in aria.h (this should replace it)
struct Aria_Module {
    char* name;
    Chunk_map_t chunks;

    Stack* stack;
    Heap heap;
};

void compileExpr(Aria_Chunk* chunk, ASTNode* node);
void compileIf(Aria_Chunk* chunk, ASTNode* node, bool inner);
void compileVar(Aria_Chunk* chunk, ASTNode* node);
void compileStmt(Aria_Chunk* chunk, ASTNode* node);
Aria_Chunk compileFunc(ASTNode* node, Aria_Module* mod);
Aria_Module ariaEmitBytecode(ASTNode ast);

void printBytecode(Aria_Module* mod);
char* opcodeName(Opcode op);

#endif  // ARIA_BYTECODE_H
