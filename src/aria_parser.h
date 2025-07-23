#ifndef ARIA_PARSER_H
#define ARIA_PARSER_H

#include <stdbool.h>
#include <stdlib.h>

#include "aria_lexer.h"

typedef enum {
    // Primary 
    AST_NUMBER, 
    AST_IDENTIFIER,
    AST_STRING,
    AST_BOOL, 
 
    // Ops
    AST_BINARY,
} NodeType;

typedef union {
    bool boolean;
    float number;
    char* text;    // must be null-terminated
} Aria_Value;
 
typedef struct Aria_ASTNode {
    bool sentinal;
    NodeType type;
    union {
        struct {
            TokenType op;
            struct Aria_ASTNode* lhs;
            struct Aria_ASTNode* rhs;
        } binary;

        // union switch on NodeType 
        struct {
            Aria_Value val;
        } literal;
    } as;
    
    struct Aria_ASTNode* next;
    struct Aria_ASTNode* prev;
} Aria_ASTNode;

// VM Interface
typedef struct {
    // Memory management
    Aria_ASTNode* ast_root;
    
    // Variable storage (hash table placeholder)
    void* globals;
    
    // Stack for function calls and local variables
    void* call_stack;
    size_t stack_size;
    size_t stack_capacity;
    
    // Garbage collection
    Aria_ASTNode** gc_objects;
    size_t gc_count;
    size_t gc_capacity;
    
    // Error handling
    bool has_error;
    char error_message[256];
    
    // Runtime state
    bool running;
} Aria_VM;

Aria_ASTNode* parseExpression(Aria_Lexer* l);
void printAST(Aria_ASTNode* node, int depth);

#endif // ARIA_PARSER_H
