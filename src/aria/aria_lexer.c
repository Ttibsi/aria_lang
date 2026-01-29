#include "aria_lexer.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/nob.h"

void ariaLexerInit(AriaLexer* lexer, char* source) {
    nob_da_reserve(lexer, 32);
    lexer->source = source;
    lexer->pc = 0;
    lexer->index = 0;
}

AriaToken makeToken(TokenType type, int start, int length) {
    AriaToken tok = (AriaToken){.valid = true, .type = type, .start = start, .len = length};
    return tok;
}

char peek(AriaLexer* L) { return L->source[L->pc]; }

char peekNext(AriaLexer* L) { return L->source[L->pc + 1]; }

char advanceChar(AriaLexer* L) { return L->source[L->pc++]; }

void skipWhitespace(AriaLexer* L) {
    while (isspace(peek(L))) { advanceChar(L); }
}

AriaToken scanEqualVariant(AriaLexer* L, TokenType single, TokenType equal, int start) {
    if (peekNext(L) == '=') {
        advanceChar(L);
        advanceChar(L);
        return makeToken(equal, start, 2);
    }

    advanceChar(L);
    return makeToken(single, start, 1);
}

AriaToken scanStringLiteral(AriaLexer* L, int start) {
    assert(peek(L) == '"');
    int length = 0;

    while (peekNext(L) != '"' && peekNext(L) != '\0') {
        advanceChar(L);
        length++;
    }

    if (peekNext(L) == '\0') {
        // TODO: Handle error case
        return makeToken(TOK_EOF, start, 0);
    }

    advanceChar(L);  // consume closing quote
    length++;
    assert(peek(L) == '"');
    advanceChar(L);  // Next char
    length++;

    return makeToken(TOK_STRING_LIT, start, length);
}

AriaToken scanNumber(AriaLexer* L, int start) {
    // TODO: handle other numeric formats (hex, bin, oct)
    int length = 0;

    do {
        advanceChar(L);
        length++;
    } while (isdigit(peek(L)) || peek(L) == '.');

    return makeToken(TOK_NUM_LIT, start, length);
}

AriaToken scanIdentifier(AriaLexer* L, int start) {
    int length = 0;

    do {
        length++;
        advanceChar(L);
    } while (isalnum(peek(L)) || peek(L) == '_');

    // Check if it's a keyword
    for (int i = 0; i < keyword_count; i++) {
        if (keywords[i].len == length && strncmp(L->source + start, keywords[i].kw, length) == 0) {
            return makeToken(keywords[i].tok, start, length);
        }
    }

    return makeToken(TOK_IDENTIFIER, start, length);
}

void advanceComment(AriaLexer* L) {
    while (L->source[L->pc] != '\n') { advanceChar(L); }
}

AriaToken scanToken(AriaLexer* L) {
    skipWhitespace(L);

    int start = L->pc;
    char c = peek(L);

    if (c == '\0') { return makeToken(TOK_EOF, start, 0); }
    switch (c) {
            // clang-format off
        case ',': advanceChar(L); return makeToken(TOK_COMMA, start, 1);
        case ';': advanceComment(L); return makeToken(TOK_COUNT, 0, 0);
        case ':': advanceChar(L); return makeToken(TOK_COLON, start, 1);
        case '-': advanceChar(L); return makeToken(TOK_MINUS, start, 1);
        case '+': advanceChar(L); return makeToken(TOK_PLUS, start, 1);
        case '*': advanceChar(L); return makeToken(TOK_STAR, start, 1);
        case '/': advanceChar(L); return makeToken(TOK_SLASH, start, 1);
        case '[': advanceChar(L); return makeToken(TOK_LEFT_SQUACKET, start, 1);
        case ']': advanceChar(L); return makeToken(TOK_RIGHT_SQUACKET, start, 1);
        case '(': advanceChar(L); return makeToken(TOK_LEFT_PAREN, start, 1);
        case ')': advanceChar(L); return makeToken(TOK_RIGHT_PAREN, start, 1);
        case '!': return scanEqualVariant(L, TOK_BANG, TOK_BANG_EQUAL, start);
        case '=': return scanEqualVariant(L, TOK_EQUAL, TOK_EQUAL_EQUAL, start);
        case '<': return scanEqualVariant(L, TOK_LESS, TOK_LESS_EQUAL, start);
        case '>': return scanEqualVariant(L, TOK_GREATER, TOK_GREATER_EQUAL, start);
        case '"': return scanStringLiteral(L, start);
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
        case '.':
            advanceChar(L);
            if (peek(L) == '.' && L->source[L->pc+1] == '.') {
                advanceChar(L);
                advanceChar(L);
                return makeToken(TOK_ELLIPSIS, start, 3);
            }
            return makeToken(TOK_DOT, start, 1);
            break;
    }

    //clang-format on

    if (isdigit(c)) { return scanNumber(L, start); }
    if (isalpha(c) || c == '_') { return scanIdentifier(L, start); }

    // TODO: Handle error case
    return makeToken(TOK_ERROR, start, 0);
}

void advance(AriaLexer* L) { L->index++; }

bool check(AriaLexer* L, const TokenType type) { return L->items[L->index].type == type; }

bool match(AriaLexer* L, const TokenType type) {
    if (check(L, type)) {
        advance(L);
        return true;
    }
    return false;
}

bool checkType(AriaLexer* L) {
    const TokenType curType = getCurrTokenType(L);
    if (curType == TOK_BOOL ||
        curType == TOK_CHAR ||
        curType == TOK_NIL  ||
        curType == TOK_NUM  ||
        curType == TOK_STR  ||
        curType == TOK_LIST ||
        curType == TOK_MAP  ||
        curType == TOK_NIL
    ) { return true; }

    return false;
}

bool isKeyword(const TokenType type) {
    for (int i = 0; i < keyword_count; i++) {
        if (keywords[i].tok == type) { return true; }
    }

    return false;
}

TokenType getCurrTokenType(AriaLexer* L) {
    const AriaToken* tkn = &L->items[L->index];
    if (tkn == NULL) { return TOK_ERROR; }
    return tkn->type;
}

int getTokenNumber(AriaLexer* L, size_t index) {
    const AriaToken* tkn = &L->items[index];
    if (tkn->type != TOK_NUM_LIT) return 0;

    char* num_str = malloc(tkn->len + 1);
    strncpy(num_str, L->source + tkn->start, tkn->len);
    num_str[tkn->len] = '\0';
    int result = atoi(num_str);
    free(num_str);
    return result;
}

char getTokenChar(AriaLexer* L, size_t index) {
    const AriaToken* tkn = &L->items[index];
    if (tkn->len == 0) return '\0';
    return L->source[tkn->start];
}

char* getTokenString(AriaLexer* L, size_t index) {
    const AriaToken* tkn = &L->items[index];
    char* str = malloc(sizeof(char) * tkn->len + 1);
    strncpy(str, L->source + tkn->start, tkn->len);
    str[tkn->len] = '\0';
    return str;
}

void ariaTokenize(AriaLexer* L) {
    AriaToken token = {};
    do {
        token = scanToken(L);
        if (token.type == TOK_COUNT) { continue; } // Use as a null token
        nob_da_append(L, token);
    } while (token.type != TOK_EOF);

    L->pc = 0;
}

char* tokenStr(const TokenType tok) {
    switch(tok) {
        case TOK_LEFT_PAREN: return "TOK_LEFT_PAREN";
        case TOK_RIGHT_PAREN: return "TOK_RIGHT_PAREN";
        case TOK_LEFT_SQUACKET: return "TOK_LEFT_SQUACKET";
        case TOK_RIGHT_SQUACKET: return "TOK_RIGHT_SQUACKET";
        case TOK_COMMA: return "TOK_COMMA";
        case TOK_DOT: return "TOK_DOT";
        case TOK_SEMICOLON: return "TOK_SEMICOLON";
        case TOK_COLON: return "TOK_COLON";
        case TOK_MINUS: return "TOK_MINUS";
        case TOK_PLUS: return "TOK_PLUS";
        case TOK_SLASH: return "TOK_SLASH";
        case TOK_STAR: return "TOK_STAR";
        case TOK_BANG: return "TOK_BANG";
        case TOK_BANG_EQUAL: return "TOK_BANG_EQUAL";
        case TOK_EQUAL: return "TOK_EQUAL";
        case TOK_EQUAL_EQUAL: return "TOK_EQUAL_EQUAL";
        case TOK_GREATER: return "TOK_GREATER";
        case TOK_GREATER_EQUAL: return "TOK_GREATER_EQUAL";
        case TOK_LESS: return "TOK_LESS";
        case TOK_LESS_EQUAL: return "TOK_LESS_EQUAL";
        case TOK_AND: return "TOK_AND";
        case TOK_OR: return "TOK_OR";
        case TOK_ELLIPSIS: return "TOK_ELLIPSIS";
        case TOK_BOOL: return "TOK_BOOL";
        case TOK_CHAR: return "TOK_CHAR";
        case TOK_ELSE: return "TOK_ELSE";
        case TOK_END: return "TOK_END";
        case TOK_FALSE: return "TOK_FALSE";
        case TOK_FOR: return "TOK_FOR";
        case TOK_FOREACH: return "TOK_FOREACH";
        case TOK_FUNC: return "TOK_FUNC";
        case TOK_IF: return "TOK_IF";
        case TOK_IMPORT: return "TOK_IMPORT";
        case TOK_IN: return "TOK_IN";
        case TOK_LIST: return "TOK_LIST";
        case TOK_MAP: return "TOK_MAP";
        case TOK_NIL: return "TOK_NIL";
        case TOK_NUM: return "TOK_NUM";
        case TOK_RET: return "TOK_RET";
        case TOK_STEP: return "TOK_STEP";
        case TOK_STR: return "TOK_STR";
        case TOK_THEN: return "TOK_THEN";
        case TOK_TO: return "TOK_TO";
        case TOK_TRUE: return "TOK_TRUE";
        case TOK_TYPE: return "TOK_TYPE";
        case TOK_VAR: return "TOK_VAR";
        case TOK_IDENTIFIER: return "TOK_IDENTIFIER";
        case TOK_STRING_LIT: return "TOK_STRING_LIT";
        case TOK_CHAR_LIT: return "TOK_CHAR_LIT";
        case TOK_NUM_LIT: return "TOK_NUM_LIT";
        case TOK_ERROR: return "TOK_ERROR";
        case TOK_EOF: return "TOK_EOF";
        case TOK_COUNT: return "";
    };

    return "UNKNOWN TOKEN";
}

void printTokens(AriaLexer* L) {
    printf("=== TOKENS ===\n");
    nob_da_foreach(AriaToken, x, L) {
        const size_t idx = x - L->items;
        printf("[%ld] Token: %d, start: %d, len: %d\n", idx, x->type, x->start, x->len);
    }

    printf("--- %ld tokens found ---\n\n", L->count);
}
