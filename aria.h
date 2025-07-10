#ifndef ARIA_H
#define ARIA_H

#include <stddef.h>
#include <stdint.h> 
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> 

// debug modes
// 1 - lexer 
#define ARIA_DEBUG 1 

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
    char* kw;     
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
static const int keyword_count = sizeof(keywords) / sizeof(Keyword);
 
typedef struct {
    TokenType type; 
    int start;
    int len;
} Aria_Token; 

typedef struct {
    Aria_Token* data;
    int size;
    int capacity;
    const char* source; // null-terminated
    const char* module; // null-terminated

    bool error; 
} Aria_Lexer;

typedef enum {
    AST_STR_LITERAL,
    AST_NUMERIC,
    AST_IDENTIFIER,
    AST_BRACKET_BLOCK,

    AST_NULL
} Aria_AstType;

typedef struct AstNode {
    Aria_AstType type;
    union {

        struct {} none;

        struct {
            Aria_Token* token;
        } literal;

        struct block {
            AstNode* data;
            int size;
            int capacity;
        } block;
        
        struct {
            struct AstNode* lhs;
            char op;
            struct AstNode* rhs;
        } bin_op;

    } is;
} Aria_AstNode;

typedef struct {
    Aria_AstNode* data;
    int size;
    int capacity;

    int curr_token;
} Aria_Parser;

typedef struct {
    Aria_Lexer* lexer; 
    Aria_Parser* parser;
} Aria_VM; 

// Global
Aria_VM aria_vm_init();
void aria_vm_destroy(Aria_VM* aria_vm);
int aria_interpret(Aria_VM* aria_vm, const char* module, const char* source);

// Lexer
Aria_Lexer aria_tokenize(Aria_VM* vm, const char* module, const char* source);
int aria_lexer_append(Aria_Lexer* l, const TokenType tok, const int begin, const int size);
void print_tokens(Aria_Lexer* lexer); 

// Parser
Aria_Parser aria_parse(Aria_VM* aria_vm);
Aria_AstNode aria_parse_tok(Aria_Lexer* l, int pos, Aria_Token* cur);
void aria_parser_append(Aria_Parser* p, const Aria_AstNode node);
void aria_parser_block_append(struct block* b, const Aria_AstNode node);
 

#endif // ARIA_H 
