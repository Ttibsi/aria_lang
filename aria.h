#ifndef ARIA_H
#define ARIA_H

#include <stdbool.h>
#include <stddef.h>

// Lexing
typedef enum {
    // Single-character tokens.
    TOK_LEFT_PAREN, TOK_RIGHT_PAREN,
    TOK_LEFT_BRACE, TOK_RIGHT_BRACE,
    TOK_COMMA, TOK_DOT, TOK_SEMICOLON,
    TOK_MINUS, TOK_PLUS, TOK_SLASH, TOK_STAR,

    // One or two character tokens.
    TOK_BANG, TOK_BANG_EQUAL,
    TOK_EQUAL, TOK_EQUAL_EQUAL,
    TOK_GREATER, TOK_GREATER_EQUAL,
    TOK_LESS, TOK_LESS_EQUAL,

    // digraphs
    TOK_AND, TOK_OR,

    // Keywords.
    TOK_CASE, TOK_CLASS, TOK_CONST, TOK_DEFAULT,
    TOK_ELSE, TOK_FALSE, TOK_FOR, TOK_FUNC, 
    TOK_IF, TOK_RETURN, TOK_STATIC, TOK_SWITCH,
    TOK_TRUE, TOK_VAR,

    // Literals. 
    TOK_IDENTIFIER, TOK_STRING, TOK_NUMBER,

    TOK_ERROR, TOK_EOF
} TokenType;

typedef struct {
    bool valid;
    TokenType type;
    int start;
    int len;
} Aria_Token;

typedef struct {
    const char* source;
    int pc; // program counter
    Aria_Token current_token;
} Aria_Lexer;

typedef struct {
    const char* kw;
    int len;
    TokenType tok;
} Keyword;

static const Keyword keywords[] = {
    { "case",    4,  TOK_CASE     },  // 22
    { "class",   5,  TOK_CLASS    },  // 23
    { "const",   5,  TOK_CONST    },  // 24
    { "default", 7,  TOK_DEFAULT  },  // 25
    { "else",    4,  TOK_ELSE     },  // 26
    { "false",   5,  TOK_FALSE    },  // 27
    { "for",     3,  TOK_FOR      },  // 28
    { "func",    4,  TOK_FUNC     },  // 29
    { "if",      2,  TOK_IF       },  // 30
    { "return",  6,  TOK_RETURN   },  // 31
    { "static",  6,  TOK_STATIC   },  // 32
    { "switch",  6,  TOK_SWITCH   },  // 33
    { "true",    4,  TOK_TRUE     },  // 34
    { "var",     3,  TOK_VAR      },  // 35
};
static const int keyword_count = sizeof(keywords) / sizeof(keywords[0]);

// Parsing
typedef enum {
    AST_BINARY,
} NodeType;

typedef struct Aria_ASTNode {
    bool sentinal;
    NodeType type;
    union {
        struct {
            TokenType op;
            struct Aria_ASTNode* lhs;
            struct Aria_ASTNode* rhs;
        } binary;
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

Aria_VM aria_vm_init(void);
void aria_vm_destroy(Aria_VM* vm);
int aria_interpret(Aria_VM* vm, const char* name, const char* src);

#endif // ARIA_H
