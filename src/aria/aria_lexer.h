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
    TOK_BOOL,     // 22
    TOK_CHAR,     // 23
    TOK_ELSE,     // 24
    TOK_END,      // 25
    TOK_FALSE,    // 26
    TOK_FOR,      // 27
    TOK_FOREACH,  // 28
    TOK_FUNC,     // 29
    TOK_IF,       // 30
    TOK_IMPORT,   // 31
    TOK_IN,       // 32
    TOK_NIL,      // 33
    TOK_NUM,      // 34
    TOK_RET,      // 35
    TOK_STEP,     // 36
    TOK_STR,      // 37
    TOK_THEN,     // 38
    TOK_TO,       // 39
    TOK_TRUE,     // 40
    TOK_TYPE,     // 41
    TOK_VAR,      // 42

    // Literals.
    TOK_IDENTIFIER,  // 43
    TOK_STRING_LIT,  // 44
    TOK_CHAR_LIT,    // 45
    TOK_NUM_LIT,     // 46

    TOK_ERROR,  // 47
    TOK_EOF,    // 48
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
    {"BOOL", 4, TOK_BOOL},        // 22
    {"CHAR", 4, TOK_CHAR},        // 23
    {"ELSE", 4, TOK_ELSE},        // 24
    {"END", 3, TOK_END},          // 25
    {"FALSE", 5, TOK_FALSE},      // 26
    {"FOR", 3, TOK_FOR},          // 27
    {"FOREACH", 7, TOK_FOREACH},  // 28
    {"FUNC", 4, TOK_FUNC},        // 29
    {"IF", 2, TOK_IF},            // 30
    {"IMPORT", 6, TOK_IMPORT},    // 31
    {"IN", 2, TOK_IN},            // 32
    {"NIL", 3, TOK_NIL},          // 33
    {"NUM", 3, TOK_NUM},          // 34
    {"RET", 3, TOK_RET},          // 35
    {"STEP", 4, TOK_STEP},        // 36
    {"STR", 3, TOK_STR},          // 37
    {"THEN", 4, TOK_THEN},        // 38
    {"TO", 2, TOK_TO},            // 39
    {"TRUE", 2, TOK_TRUE},        // 40
    {"TYPE", 4, TOK_TYPE},        // 41
    {"VAR", 3, TOK_VAR},          // 42
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
void advanceComment(AriaLexer* L);  // TODO: test
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
