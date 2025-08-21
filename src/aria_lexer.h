#ifndef ARIA_LEXER_H
#define ARIA_LEXER_H

#include <stdbool.h>

typedef enum {
    // Single-character tokens.
    TOK_LEFT_PAREN,     // 0
    TOK_RIGHT_PAREN,    // 1
    TOK_LEFT_BRACE,     // 2
    TOK_RIGHT_BRACE,    // 3
    TOK_COMMA,          // 4
    TOK_DOT,            // 5
    TOK_SEMICOLON,      // 6
    TOK_MINUS,          // 7
    TOK_PLUS,           // 8
    TOK_SLASH,          // 9
    TOK_STAR,           // 10

    // One or two character tokens.
    TOK_BANG,           // 11
    TOK_BANG_EQUAL,     // 12
    TOK_EQUAL,          // 13
    TOK_EQUAL_EQUAL,    // 14
    TOK_GREATER,        // 15
    TOK_GREATER_EQUAL,  // 16
    TOK_LESS,           // 17
    TOK_LESS_EQUAL,     // 18

    // digraphs
    TOK_AND,            // 19
    TOK_OR,             // 20

    // Keywords.
    TOK_CASE,           // 21
    TOK_CLASS,          // 22
    TOK_CONST,          // 23
    TOK_DEFAULT,        // 24
    TOK_ELSE,           // 25
    TOK_FALSE,          // 26
    TOK_FOR,            // 27
    TOK_FUNC,           // 28
    TOK_IF,             // 29
    TOK_RETURN,         // 30
    TOK_STATIC,         // 31
    TOK_SWITCH,         // 32
    TOK_TRUE,           // 33
    TOK_VAR,            // 34

    // Literals.
    TOK_IDENTIFIER,     // 35
    TOK_STRING,         // 36
    TOK_NUMBER,         // 37

    TOK_ERROR,          // 38
    TOK_EOF             // 39
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
    { "case",    4,  TOK_CASE     },  // 21
    { "class",   5,  TOK_CLASS    },  // 22
    { "const",   5,  TOK_CONST    },  // 23
    { "default", 7,  TOK_DEFAULT  },  // 24
    { "else",    4,  TOK_ELSE     },  // 25
    { "false",   5,  TOK_FALSE    },  // 26
    { "for",     3,  TOK_FOR      },  // 27
    { "func",    4,  TOK_FUNC     },  // 28
    { "if",      2,  TOK_IF       },  // 29
    { "return",  6,  TOK_RETURN   },  // 30
    { "static",  6,  TOK_STATIC   },  // 31
    { "switch",  6,  TOK_SWITCH   },  // 32
    { "true",    4,  TOK_TRUE     },  // 33
    { "var",     3,  TOK_VAR      },  // 34
};
static const int keyword_count = sizeof(keywords) / sizeof(keywords[0]);

bool match(Aria_Lexer* l, TokenType type);
Aria_Token scanToken(Aria_Lexer* l);
void advance(Aria_Lexer* l);
int get_token_number(Aria_Lexer* lexer, Aria_Token token);
char get_token_char(Aria_Lexer* lexer, Aria_Token token);

#endif // ARIA_LEXER_H
