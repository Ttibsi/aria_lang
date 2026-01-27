#ifndef ARIA_LEXER_H
#define ARIA_LEXER_H

#include <stddef.h>
typedef enum {
    // Single-character tokens.
    TOK_LEFT_PAREN,      // 0
    TOK_RIGHT_PAREN,     // 1
    TOK_LEFT_SQUACKET,   // 2
    TOK_RIGHT_SQUACKET,  // 3
    TOK_COMMA,           // 4
    TOK_DOT,             // 5
    TOK_SEMICOLON,       // 6
    TOK_COLON,           // 7
    TOK_MINUS,           // 8
    TOK_PLUS,            // 9
    TOK_SLASH,           // 10
    TOK_STAR,            // 11

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
    TOK_AND,       // 20
    TOK_OR,        // 21
    TOK_ELLIPSIS,  // 22

    // Keywords.
    TOK_BOOL,     // 23
    TOK_CHAR,     // 24
    TOK_ELSE,     // 25
    TOK_END,      // 26
    TOK_FALSE,    // 27
    TOK_FOR,      // 28
    TOK_FOREACH,  // 29
    TOK_FUNC,     // 30
    TOK_IF,       // 31
    TOK_IMPORT,   // 32
    TOK_IN,       // 33
    TOK_LIST,     // 34
    TOK_MAP,      // 35
    TOK_NIL,      // 36
    TOK_NUM,      // 37
    TOK_RET,      // 38
    TOK_STEP,     // 39
    TOK_STR,      // 40
    TOK_THEN,     // 41
    TOK_TO,       // 42
    TOK_TRUE,     // 43
    TOK_TYPE,     // 44
    TOK_VAR,      // 45

    // Literals.
    TOK_IDENTIFIER,  // 46
    TOK_STRING_LIT,  // 47
    TOK_CHAR_LIT,    // 48
    TOK_NUM_LIT,     // 49

    TOK_ERROR,  // 50
    TOK_EOF,    // 51
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
    {"BOOL", 4, TOK_BOOL},        // 23
    {"CHAR", 4, TOK_CHAR},        // 24
    {"ELSE", 4, TOK_ELSE},        // 25
    {"END", 3, TOK_END},          // 26
    {"FALSE", 5, TOK_FALSE},      // 27
    {"FOR", 3, TOK_FOR},          // 28
    {"FOREACH", 7, TOK_FOREACH},  // 29
    {"FUNC", 4, TOK_FUNC},        // 30
    {"IF", 2, TOK_IF},            // 31
    {"IMPORT", 6, TOK_IMPORT},    // 32
    {"IN", 2, TOK_IN},            // 33
    {"LIST", 4, TOK_LIST},        // 34
    {"MAP", 3, TOK_MAP},          // 35
    {"NIL", 3, TOK_NIL},          // 36
    {"NUM", 3, TOK_NUM},          // 37
    {"RET", 3, TOK_RET},          // 38
    {"STEP", 4, TOK_STEP},        // 39
    {"STR", 3, TOK_STR},          // 40
    {"THEN", 4, TOK_THEN},        // 41
    {"TO", 2, TOK_TO},            // 42
    {"TRUE", 2, TOK_TRUE},        // 43
    {"TYPE", 4, TOK_TYPE},        // 44
    {"VAR", 3, TOK_VAR},          // 45
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
AriaToken scanEqualVariant(AriaLexer* L, TokenType single, TokenType equal, int start);
AriaToken scanStringLiteral(AriaLexer* L, int start);
AriaToken scanNumber(AriaLexer* L, int start);
AriaToken scanIdentifier(AriaLexer* L, int start);
void advanceComment(AriaLexer* L);
AriaToken scanToken(AriaLexer* L);
void advance(AriaLexer* L);
bool check(AriaLexer* L, const TokenType type);
bool match(AriaLexer* L, const TokenType type);
bool checkType(AriaLexer* L);
bool isKeyword(const TokenType type);
TokenType getCurrTokenType(AriaLexer* L);
int getTokenNumber(AriaLexer* L, size_t index);
char getTokenChar(AriaLexer* L, size_t index);
char* getTokenString(AriaLexer* L, size_t index);
void ariaTokenize(AriaLexer* L);
void printTokens(AriaLexer* L);

#endif  // ARIA_LEXER_H
