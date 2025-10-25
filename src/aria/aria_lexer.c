#include "aria_lexer.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>

Aria_Lexer lexerInit(char* text) {
    return (Aria_Lexer){
        .source = strdup(text),
        .pc = 0,
        .tokens = bufferCreate(sizeof(Aria_Token), 32),
        .buf_index = 0
    };
}

Aria_Token makeToken(TokenType type, int start, int length) {
    return (Aria_Token){
        .valid = true,
        .type = type,
        .start = start,
        .len = length
    };
}

char peek(Aria_Lexer* L) {
    return L->source[L->pc];
}

char peekNext(Aria_Lexer* L) {
    return L->source[L->pc + 1];
}

char advanceChar(Aria_Lexer* L) {
    return L->source[L->pc++];
}

void skipWhitespace(Aria_Lexer* L) {
    while (isspace(peek(L))) {
        advanceChar(L);
    }
}

Aria_Token scanEqualVariant(Aria_Lexer* L, TokenType single, TokenType equal) {
    // int start = L->pc - 1;
    if (peekNext(L) == '=') {
        advanceChar(L);
        return makeToken(equal, L->pc, 2);
    }
    return makeToken(single, L->pc, 1);
}

Aria_Token scanStringLiteral(Aria_Lexer* L) {
    assert (peek(L) == '"');
    int start = L->pc;
    int length = 0;

    while (peekNext(L) != '"' && peekNext(L) != '\0') {
        advanceChar(L);
        length++;
    }

    if (peekNext(L) == '\0') {
        // TODO: Handle error case
        return makeToken(TOK_EOF, start, 0);
    }

    advanceChar(L); // consume closing quote
    length++;
    assert (peek(L) == '"');
    advanceChar(L); // Next char
    length++;

    return makeToken(TOK_STRING, start, length);
}

Aria_Token scanNumber(Aria_Lexer* L) {
    int start = (L->pc - 1 > 0) ? L->pc : 0;
    int length = 0;

    do {
        advanceChar(L);
        length++;
    } while (isdigit(peek(L)));

    return makeToken(TOK_NUMBER, start, length);
}

Aria_Token scanIdentifier(Aria_Lexer* L) {
    int start = (L->pc - 1 > 0) ? L->pc : 0;
    int length = 0;

    do {
        length++;
        advanceChar(L);
    } while (isalnum(peek(L)) || peek(L) == '_');

    // Check if it's a keyword
    for (int i = 0; i < keyword_count; i++) {
        if (
            keywords[i].len == length &&
            strncmp(L->source + start, keywords[i].kw, length) == 0
        ) {
            return makeToken(keywords[i].tok, start, length);
        }
    }

    return makeToken(TOK_IDENTIFIER, start, length);
}

Aria_Token scanToken(Aria_Lexer* L) {
    skipWhitespace(L);

    int start = L->pc;
    char c = peek(L);

    if (c == '\0') { return makeToken(TOK_EOF, start, 0); }
    switch (c) {
        case '.': advanceChar(L); return makeToken(TOK_DOT, start, 1);
        case ',': advanceChar(L); return makeToken(TOK_COMMA, start, 1);
        case ';': advanceChar(L); return makeToken(TOK_SEMICOLON, start, 1);
        case '-': advanceChar(L); return makeToken(TOK_MINUS, start, 1);
        case '+': advanceChar(L); return makeToken(TOK_PLUS, start, 1);
        case '*': advanceChar(L); return makeToken(TOK_STAR, start, 1);
        case '/': advanceChar(L); return makeToken(TOK_SLASH, start, 1);
        case '{': advanceChar(L); return makeToken(TOK_LEFT_BRACE, start, 1);
        case '}': advanceChar(L); return makeToken(TOK_RIGHT_BRACE, start, 1);
        case '(': advanceChar(L); return makeToken(TOK_LEFT_PAREN, start, 1);
        case ')': advanceChar(L); return makeToken(TOK_RIGHT_PAREN, start, 1);
        case '!': advanceChar(L); return scanEqualVariant(L, TOK_BANG, TOK_BANG_EQUAL);
        case '=': advanceChar(L); return scanEqualVariant(L, TOK_EQUAL, TOK_EQUAL_EQUAL);
        case '<': advanceChar(L); return scanEqualVariant(L, TOK_LESS, TOK_LESS_EQUAL);
        case '>': advanceChar(L); return scanEqualVariant(L, TOK_GREATER, TOK_GREATER_EQUAL);
        case '"': return scanStringLiteral(L);
        case '&':
            if (peek(L) == '&') {
                advanceChar(L);
                advanceChar(L);
                return makeToken(TOK_AND, start, 2);
            }
            break;
        case '|':
            if (peek(L) == '|') {
                advanceChar(L);
                advanceChar(L);
                return makeToken(TOK_OR, start, 2);
            }
            break;
    }

    if (isdigit(c)) { return scanNumber(L); }
    if (isalpha(c) || c == '_') { return scanIdentifier(L); }

    // TODO: Handle error case
    return makeToken(TOK_ERROR, start, 0);
}

void advance(Aria_Lexer* L) {
    L->buf_index++;
}

bool check(Aria_Lexer* L, const TokenType type) {
    Aria_Token* tkn = bufferGet(L->tokens, L->buf_index);
    return tkn->type == type;
}

bool match(Aria_Lexer* L, const TokenType type) {
    if (check(L, type)) {
        advance(L);
        return true;
    }
    return false;
}

TokenType getCurrTokenType(Aria_Lexer* L) {
    const Aria_Token* tkn = bufferGet(L->tokens, L->buf_index);
    if (tkn == NULL) { return TOK_ERROR; }
    return tkn->type;
}

// TODO: Refactor these two functionss to take in an index instead of a token
int getTokenNumber(Aria_Lexer* L, Aria_Token* token) {
    if (token->type != TOK_NUMBER) return 0;

    char* num_str = malloc(token->len + 1);
    strncpy(num_str, L->source + token->start, token->len);
    num_str[token->len] = '\0';
    int result = atoi(num_str);
    free(num_str);
    return result;
}

char getTokenChar(Aria_Lexer* L, Aria_Token* token) {
    if (token->len == 0) return '\0';
    return L->source[token->start];
}

char* getTokenString(Aria_Lexer* L, size_t index) {
    Aria_Token* tkn = (Aria_Token*)bufferGet(L->tokens, index);
    char* str = malloc(sizeof(char) * tkn->len + 1);
    strncpy(str, L->source + tkn->start, tkn->len);
    str[tkn->len] = '\0';
    return str;
}

void ariaTokenize(Aria_Lexer* L) {
    Aria_Token token = {};
    do {
        token = scanToken(L);
        bufferInsert(L->tokens, &token);
    } while(token.type != TOK_EOF);

    L->pc = 0;
}

void printTokens(Aria_Lexer* L) {
    printf("=== TOKENS ===\n");
    for (uint32_t i = 0; i < L->tokens->size; i++) {
        Aria_Token* tok = bufferGet(L->tokens, i);
        printf("[%d] Token: %d, start: %d, len: %d\n", i, tok->type, tok->start, tok->len);
    }
    printf("--- %d tokens found ---\n", L->tokens->size);
}
