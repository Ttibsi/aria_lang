#include "aria_lexer.h"

#include <ctype.h>
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

Aria_Token makeToken(Aria_Lexer* l, TokenType type, int start, int length) {
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
        return makeToken(l, equal, start, 2);
    }
    return makeToken(l, single, start, 1);
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
        return makeToken(l, TOK_EOF, start, 0);
    }

    advanceChar(l); // consume closing quote
    return makeToken(l, TOK_STRING, start, length + 2);
}

Aria_Token scanNumber(Aria_Lexer* l) {
    int start = l->pc - 1;
    int length = 0;

    do {
        advanceChar(l);
        length++;
    } while (isdigit(peek(l)));

    return makeToken(l, TOK_NUMBER, start, length);
}

Aria_Token scanIdentifier(Aria_Lexer* l) {
    int start = l->pc;
    int length = 0;

    while (isalnum(peek(l)) || peek(l) == '_') {
        advanceChar(l);
        length++;
    }

    // Check if it's a keyword
    for (int i = 0; i < keyword_count; i++) {
        if (keywords[i].len == length &&
            strncmp(l->source + start, keywords[i].kw, length) == 0) {
            return makeToken(l, keywords[i].tok, start, length);
        }
    }

    return makeToken(l, TOK_IDENTIFIER, start, length);
}

Aria_Token scanToken(Aria_Lexer* l) {
    skipWhitespace(l);

    char c = advanceChar(l);
    int start = l->pc - 1;

    if (c == '\0') {
        return makeToken(l, TOK_EOF, start, 0);
    }

    switch (c) {
        case '.': return makeToken(l, TOK_DOT, start, 1);
        case ',': return makeToken(l, TOK_COMMA, start, 1);
        case ';': return makeToken(l, TOK_SEMICOLON, start, 1);
        case '-': return makeToken(l, TOK_MINUS, start, 1);
        case '+': return makeToken(l, TOK_PLUS, start, 1);
        case '*': return makeToken(l, TOK_STAR, start, 1);
        case '/': return makeToken(l, TOK_SLASH, start, 1);
        case '{': return makeToken(l, TOK_LEFT_BRACE, start, 1);
        case '}': return makeToken(l, TOK_RIGHT_BRACE, start, 1);
        case '(': return makeToken(l, TOK_LEFT_PAREN, start, 1);
        case ')': return makeToken(l, TOK_RIGHT_PAREN, start, 1);
        case '!': return scanEqualVariant(l, TOK_BANG, TOK_BANG_EQUAL);
        case '=': return scanEqualVariant(l, TOK_EQUAL, TOK_EQUAL_EQUAL);
        case '<': return scanEqualVariant(l, TOK_LESS, TOK_LESS_EQUAL);
        case '>': return scanEqualVariant(l, TOK_GREATER, TOK_GREATER_EQUAL);
        case '&':
            if (peek(l) == '&') {
                advanceChar(l);
                return makeToken(l, TOK_AND, start, 2);
            }
            break;
        case '|':
            if (peek(l) == '|') {
                advanceChar(l);
                return makeToken(l, TOK_OR, start, 2);
            }
            break;
        case '"': return scanStringLiteral(l);
    }

    if (isdigit(c)) {
        return scanNumber(l);
    }

    if (isalpha(c) || c == '_') {
        return scanIdentifier(l);
    }

    // TODO: Handle error case
    return makeToken(l, TOK_EOF, start, 0);
}

void advance(Aria_Lexer* l) {
    l->current_token = scanToken(l);
}

bool check(Aria_Lexer* l, TokenType type) {
    return l->current_token.type == type;
}

bool match(Aria_Lexer* l, TokenType type) {
    if (check(l, type)) {
        advance(l);
        return true;
    }
    return false;
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
