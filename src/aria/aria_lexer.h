#ifndef ARIA_LEXER_H
#define ARIA_LEXER_H

#include "aria_buffer.h"

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
    TOK_EXPORT,         // 26
    TOK_FALSE,          // 27
    TOK_FOR,            // 28
    TOK_FUNC,           // 29
    TOK_IF,             // 30
    TOK_IMPORT,         // 31
    TOK_RETURN,         // 33
    TOK_STATIC,         // 34
    TOK_SWITCH,         // 35
    TOK_TRUE,           // 36
    TOK_VAR,            // 37

    // Literals.
    TOK_IDENTIFIER,     // 38
    TOK_STRING,         // 39
    TOK_NUMBER,         // 40

    TOK_ERROR,          // 41
    TOK_EOF             // 42
} TokenType;

typedef struct _Aria_Token {
    bool valid;
    TokenType type;
    int start;
    int len;
} Aria_Token;

typedef struct {
    char* source;
    int pc; // program counter
    Aria_Buffer* tokens;
    int buf_index;
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
    { "export",  6,  TOK_EXPORT   },  // 26
    { "false",   5,  TOK_FALSE    },  // 27
    { "for",     3,  TOK_FOR      },  // 28
    { "func",    4,  TOK_FUNC     },  // 29
    { "if",      2,  TOK_IF       },  // 30
    { "import",  6,  TOK_IMPORT   },  // 31
    { "return",  6,  TOK_RETURN   },  // 33
    { "static",  6,  TOK_STATIC   },  // 34
    { "switch",  6,  TOK_SWITCH   },  // 35
    { "true",    4,  TOK_TRUE     },  // 36
    { "var",     3,  TOK_VAR      },  // 37
};
static const int keyword_count = sizeof(keywords) / sizeof(keywords[0]);

Aria_Lexer lexerInit(char* text);
Aria_Token makeToken(TokenType type, int start, int length);
char peek(Aria_Lexer* L);
char peekNext(Aria_Lexer* L);
char advanceChar(Aria_Lexer* L);
void skipWhitespace(Aria_Lexer* L);
Aria_Token scanEqualVariant(Aria_Lexer* L, TokenType single, TokenType equal);
Aria_Token scanStringLiteral(Aria_Lexer* L);
Aria_Token scanNumber(Aria_Lexer* L);
Aria_Token scanIdentifier(Aria_Lexer* L);
Aria_Token scanToken(Aria_Lexer* L);
void advance(Aria_Lexer* L);
bool check(Aria_Lexer* L, const TokenType type);
bool match(Aria_Lexer* L, const TokenType type);
TokenType getCurrTokenType(Aria_Lexer* L);
int getTokenNumber(Aria_Lexer* L, Aria_Token* token);
char getTokenChar(Aria_Lexer* L, Aria_Token* token);
void ariaTokenize(Aria_Lexer* L);
void printTokens(Aria_Lexer* L);

#endif // ARIA_LEXER_H
