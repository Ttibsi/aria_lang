#define ARIA_BUFFER_IMPL
#include "aria_lexer.h"
#include "aria_buffer.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char peek(Aria_Lexer* l) {
    return l->source[l->pc];
}

char peekNext(Aria_Lexer* l) {
    return l->source[l->pc + 1];
}

char advanceChar(Aria_Lexer* l) {
    return l->source[l->pc++];
}

void skipWhitespace(Aria_Lexer* l) {
    while (isspace(peek(l))) {
        advanceChar(l);
    }
}

Aria_Token makeToken(TokenType type, int start, int length) {
    return (Aria_Token){
        .valid = true,
        .type = type,
        .start = start,
        .len = length
    };
}

Aria_Token scanEqualVariant(Aria_Lexer* l, TokenType single, TokenType equal) {
    int start = l->pc - 1;
    if (peek(l) == '=') {
        advanceChar(l);
        return makeToken(equal, start, 2);
    }
    return makeToken(single, start, 1);
}

Aria_Token scanStringLiteral(Aria_Lexer* l) {
    int start = l->pc - 1;
    int length = 0;

    while (peek(l) != '"' && peek(l) != '\0') {
        advanceChar(l);
        length++;
    }

    if (peek(l) == '\0') {
        // TODO: Handle error case
        return makeToken(TOK_EOF, start, 0);
    }

    advanceChar(l); // consume closing quote
    return makeToken(TOK_STRING, start, length + 2);
}

Aria_Token scanNumber(Aria_Lexer* l) {
    int start = (l->pc - 1 > 0) ? l->pc : 0;
    int length = 0;

    do {
        advanceChar(l);
        length++;
    } while (isdigit(peek(l)));

    return makeToken(TOK_NUMBER, start, length);
}

Aria_Token scanIdentifier(Aria_Lexer* l) {
    int start = (l->pc - 1 > 0) ? l->pc : 0;
    int length = 0;

    do {
        length++;
        advanceChar(l);
    } while (isalnum(peek(l)) || peek(l) == '_');

    // Check if it's a keyword
    for (int i = 0; i < keyword_count; i++) {
        if (keywords[i].len == length &&
            strncmp(l->source + start, keywords[i].kw, length) == 0) {
            return makeToken(keywords[i].tok, start, length);
        }
    }

    return makeToken(TOK_IDENTIFIER, start, length);
}

Aria_Token scanToken(Aria_Lexer* l) {
    skipWhitespace(l);

    int start = l->pc;
    char c = peek(l);

    if (c == '\0') { return makeToken(TOK_EOF, start, 0); }
    switch (c) {
        case '.': advanceChar(l); return makeToken(TOK_DOT, start, 1);
        case ',': advanceChar(l); return makeToken(TOK_COMMA, start, 1);
        case ';': advanceChar(l); return makeToken(TOK_SEMICOLON, start, 1);
        case '-': advanceChar(l); return makeToken(TOK_MINUS, start, 1);
        case '+': advanceChar(l); return makeToken(TOK_PLUS, start, 1);
        case '*': advanceChar(l); return makeToken(TOK_STAR, start, 1);
        case '/': advanceChar(l); return makeToken(TOK_SLASH, start, 1);
        case '{': advanceChar(l); return makeToken(TOK_LEFT_BRACE, start, 1);
        case '}': advanceChar(l); return makeToken(TOK_RIGHT_BRACE, start, 1);
        case '(': advanceChar(l); return makeToken(TOK_LEFT_PAREN, start, 1);
        case ')': advanceChar(l); return makeToken(TOK_RIGHT_PAREN, start, 1);
        case '!': advanceChar(l); return scanEqualVariant(l, TOK_BANG, TOK_BANG_EQUAL);
        case '=': advanceChar(l); return scanEqualVariant(l, TOK_EQUAL, TOK_EQUAL_EQUAL);
        case '<': advanceChar(l); return scanEqualVariant(l, TOK_LESS, TOK_LESS_EQUAL);
        case '>': advanceChar(l); return scanEqualVariant(l, TOK_GREATER, TOK_GREATER_EQUAL);
        case '"': advanceChar(l); return scanStringLiteral(l);
        case '&':
            if (peek(l) == '&') {
                advanceChar(l);
                advanceChar(l);
                return makeToken(TOK_AND, start, 2);
            }
            break;
        case '|':
            if (peek(l) == '|') {
                advanceChar(l);
                advanceChar(l);
                return makeToken(TOK_OR, start, 2);
            }
            break;
    }

    if (isdigit(c)) { return scanNumber(l); }
    if (isalpha(c) || c == '_') { return scanIdentifier(l); }

    // TODO: Handle error case
    return makeToken(TOK_ERROR, start, 0);
}

void advance(Aria_Lexer* l) {
    l->buf_index++;
}

bool check(Aria_Lexer* l, TokenType type) {
    return ((Aria_Token*)bufferGet(l->tokens, l->buf_index))->type == type;
}

bool match(Aria_Lexer* l, TokenType type) {
    if (check(l, type)) {
        advance(l);
        return true;
    }
    return false;
}

TokenType getCurrTokenType(Aria_Lexer *l) {
    return ((Aria_Token*)bufferGet(l->tokens, l->buf_index))->type;
}

int getTokenNumber(Aria_Lexer* lexer, Aria_Token token) {
    if (token.type != TOK_NUMBER) return 0;

    char* num_str = malloc(token.len + 1);
    strncpy(num_str, lexer->source + token.start, token.len);
    num_str[token.len] = '\0';
    int result = atoi(num_str);
    free(num_str);
    return result;
}

char getTokenChar(Aria_Lexer* lexer, Aria_Token token) {
    if (token.len == 0) return '\0';
    return lexer->source[token.start];
}

void printTokens(Aria_Lexer* l) {
    printf("=== TOKENS ===\n");
    for (uint32_t i = 0; i < l->tokens->size; i++) {
        Aria_Token* tok = bufferGet(l->tokens, i);
        printf("Token: %d, start: %d, len: %d\n", tok->type, tok->start, tok->len);
    }
}

Aria_Lexer* ariaTokenize(char* src) {
    Aria_Lexer* lexer = malloc(sizeof(Aria_Lexer));
    lexer->source = malloc(strlen(src) + 1);
    strcpy(lexer->source, src);
    lexer->pc = 0;
    lexer->tokens = bufferCreate(sizeof(Aria_Token), 64);
    lexer->buf_index = 0;

    do {
        Aria_Token token = scanToken(lexer);
        bufferInsert(lexer->tokens, &token);
    } while(((Aria_Token*)bufferPeek(lexer->tokens))->type != TOK_EOF);

    lexer->pc = 0;
    return lexer;
}
