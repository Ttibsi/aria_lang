#ifndef ARIA_H
#define ARIA_H

#include <stddef.h>
#include <stdint.h> 
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> 

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
    { "case",    4,  TOK_CASE     },
    { "class",   5,  TOK_CLASS    }, 
    { "const",   5,  TOK_CONST    }, 
    { "default", 7,  TOK_DEFAULT  }, 
    { "else",    4,  TOK_ELSE     }, 
    { "false",   5,  TOK_FALSE    }, 
    { "for",     3,  TOK_FOR      }, 
    { "func",    4,  TOK_FUNC     },
    { "if",      2,  TOK_IF       }, 
    { "return",  6,  TOK_RETURN   }, 
    { "static",  6,  TOK_STATIC   }, 
    { "switch",  6,  TOK_SWITCH   }, 
    { "true",    4,  TOK_TRUE     }, 
    { "var",     3,  TOK_VAR      },
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

typedef struct {
    Aria_Lexer* lexer; 
} Aria_VM; 

// Global
Aria_VM aria_vm_init();
void aria_vm_destroy(Aria_VM* aria_vm);
int aria_interpret(Aria_VM* aria_vm, const char* module, const char* source);

// Lexer
Aria_Lexer aria_tokenize(Aria_VM* vm, const char* module, const char* source);
void aria_lexer_append(Aria_Lexer* lexer, Aria_Token* tok);

#endif // ARIA_H 
