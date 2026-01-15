#ifndef ARIA_LEXER_H
#define ARIA_LEXER_H

#include <stddef.h>
typedef enum {
    // Single-character tokens.
    TOK_LEFT_PAREN,   // 0
    TOK_RIGHT_PAREN,  // 1
    TOK_LEFT_BRACE,   // 2
    TOK_RIGHT_BRACE,  // 3
    TOK_COMMA,        // 4
    TOK_DOT,          // 5
    TOK_SEMICOLON,    // 6
    TOK_COLON,        // 7
    TOK_MINUS,        // 8
    TOK_PLUS,         // 9
    TOK_SLASH,        // 10
    TOK_STAR,         // 11

    // One or two character tokens.
    TOK_BANG,           // 12
    TOK_BANG_EQUAL,     // 13
    TOK_EQUAL,          // 14
    TOK_EQUAL_EQUAL,    // 15
    TOK_GREATER,        // 16
    TOK_GREATER_EQUAL,  // 17
    TOK_LESS,           // 18
    TOK_LESS_EQUAL,     // 19

    // digraphs
    TOK_AND,  // 20
    TOK_OR,   // 21

    // Keywords.
    TOK_CASE,     // 22
    TOK_DEFAULT,  // 23
    TOK_ELSE,     // 24
    TOK_FALSE,    // 25
    TOK_FOR,      // 26
    TOK_FUNC,     // 27
    TOK_IF,       // 28
    TOK_IMPORT,   // 29
    TOK_NIL,      // 30
    TOK_RETURN,   // 31
    TOK_STRUCT,   // 32
    TOK_SWITCH,   // 33
    TOK_TRUE,     // 34
    TOK_VAR,      // 35

    // Literals.
    TOK_IDENTIFIER,  // 36
    TOK_STRING,      // 37
    TOK_CHAR,        // 38
    TOK_NUMBER,      // 39

    TOK_ERROR,  // 40
    TOK_EOF,    // 41
    TOK_COUNT
} TokenType;

typedef struct {
    bool valid;
    TokenType type;
    int start;
    int len;
} AriaToken;

typedef struct {
    const char* kw;
    int len;
    TokenType tok;
} AriaKeyword;

static const AriaKeyword keywords[] = {
    {"case", 4, TOK_CASE},        // 22
    {"default", 7, TOK_DEFAULT},  // 23
    {"else", 4, TOK_ELSE},        // 24
    {"false", 5, TOK_FALSE},      // 25
    {"for", 3, TOK_FOR},          // 26
    {"func", 4, TOK_FUNC},        // 27
    {"if", 2, TOK_IF},            // 28
    {"import", 6, TOK_IMPORT},    // 29
    {"nil", 3, TOK_NIL},          // 30
    {"return", 6, TOK_RETURN},    // 31
    {"struct", 6, TOK_STRUCT},    // 32
    {"switch", 6, TOK_SWITCH},    // 33
    {"true", 4, TOK_TRUE},        // 34
    {"var", 3, TOK_VAR},          // 35
};
static const int keyword_count = sizeof(keywords) / sizeof(keywords[0]);

typedef struct {
    // NOTE: dynamic array to be used with the nob_da_append macros
    AriaToken* items;
    size_t count;
    size_t capacity;

    char* source;
    size_t pc;

    // For indexing into the dynamic array when needed;
    size_t index;
} AriaLexer;

void ariaLexerInit(AriaLexer* lexer, char* source);
AriaToken makeToken(TokenType type, int start, int length);
char peek(AriaLexer* L);
char peekNext(AriaLexer* L);
char advanceChar(AriaLexer* L);
void skipWhitespace(AriaLexer* L);
AriaToken scanEqualVariant(AriaLexer* L, TokenType single, TokenType equal);
AriaToken scanStringLiteral(AriaLexer* L);
AriaToken scanNumber(AriaLexer* L);
AriaToken scanIdentifier(AriaLexer* L);
AriaToken scanToken(AriaLexer* L);
void advance(AriaLexer* L);
bool check(AriaLexer* L, const TokenType type);
bool match(AriaLexer* L, const TokenType type);
TokenType getCurrTokenType(AriaLexer* L);
int getTokenNumber(AriaLexer* L, size_t index);
char getTokenChar(AriaLexer* L, size_t index);
char* getTokenString(AriaLexer* L, size_t index);
void ariaTokenize(AriaLexer* L);
void printTokens(AriaLexer* L);

#endif  // ARIA_LEXER_H
