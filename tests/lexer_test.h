#include <stdbool.h>

#include "aria/aria_lexer.h"
#include "onetest.h"

static inline int test_ariaLexerInit(void) {
    AriaLexer L = {0};
    ariaLexerInit(&L, "3 + 4");
    onetest_assert(strcmp(L.source, "3 + 4") == 0);
    onetest_assert(L.count == 0);
    return 0;
}

static inline int test_makeToken(void) {
    AriaToken tkn = makeToken(TOK_PLUS, 2, 1);
    onetest_assert(tkn.valid == true);
    onetest_assert(tkn.type == TOK_PLUS);
    onetest_assert(tkn.start == 2);
    onetest_assert(tkn.len == 1);
    return 0;
}

static inline int test_peek(void) {
    AriaLexer L = {0};
    ariaLexerInit(&L, "3 + 4");
    ariaTokenize(&L);

    const char actual = peek(&L);
    onetest_assert(actual == '3');
    return 0;
}

static inline int test_peekNext(void) {
    AriaLexer L = {0};
    ariaLexerInit(&L, "3 + 4");
    ariaTokenize(&L);

    char actual = peekNext(&L);
    onetest_assert(actual == ' ');
    return 0;
}

static inline int test_advanceChar(void) {
    AriaLexer L = {0};
    ariaLexerInit(&L, "3 + 4");
    ariaTokenize(&L);

    char actual = advanceChar(&L);
    onetest_assert(actual == '3');
    actual = advanceChar(&L);
    onetest_assert(actual == ' ');
    return 0;
}

static inline int test_skipWhitespace(void) {
    AriaLexer L = {0};
    ariaLexerInit(&L, "3 + 4");
    ariaTokenize(&L);

    advanceChar(&L);
    skipWhitespace(&L);
    onetest_assert(peek(&L) == '+');
    return 0;
}

static inline int test_scanEqualVariant(void) {
    AriaLexer L = {0};
    ariaLexerInit(&L, "!=");
    ariaTokenize(&L);

    AriaToken tok = scanEqualVariant(&L, TOK_BANG, TOK_BANG_EQUAL);
    onetest_assert(tok.type == TOK_BANG_EQUAL);

    L.source = "<";
    tok = scanEqualVariant(&L, TOK_LESS, TOK_LESS_EQUAL);
    onetest_assert(tok.type == TOK_LESS);

    return 0;
}

static inline int test_scanStringLiteral(void) {
    AriaLexer L = {0};
    ariaLexerInit(&L, "\"hello world 123!@#\"");
    ariaTokenize(&L);
    advance(&L);

    // Test 1: standard string
    AriaToken tok1 = scanStringLiteral(&L);
    onetest_assert(tok1.valid == true);
    onetest_assert(tok1.type == TOK_STRING);
    onetest_assert(tok1.start == 0);
    onetest_assert(tok1.len == 20);
    onetest_assert(L.pc == 20);  // should be positioned after closing quote

    // Test 2 - Empty string
    L.source = "\"\"";
    L.pc = 0;

    AriaToken tok2 = scanStringLiteral(&L);
    onetest_assert(tok2.valid == true);
    onetest_assert(tok2.type == TOK_STRING);
    onetest_assert(tok2.start == 0);
    onetest_assert(tok2.len == 2);  // just the two quotes
    onetest_assert(L.pc == 2);      // should be positioned after closing quote

    // Test 3 - Unterminated string
    L.source = "\"unterminated";
    L.pc = 0;

    AriaToken tok4 = scanStringLiteral(&L);
    onetest_assert(tok4.valid == true);
    onetest_assert(tok4.type == TOK_EOF);  // function returns EOF for error case
    onetest_assert(tok4.start == 0);
    onetest_assert(tok4.len == 0);

    return 0;
}

static inline int test_scanNumber(void) {
    AriaLexer L = {0};
    ariaLexerInit(&L, "123 + 45");
    ariaTokenize(&L);

    AriaToken tok = scanNumber(&L);
    onetest_assert(tok.type == TOK_NUMBER);
    onetest_assert(tok.len == 3);

    return 0;
}

static inline int test_scanIdentifier(void) {
    AriaLexer L = {0};
    ariaLexerInit(&L, "var temp");
    ariaTokenize(&L);

    AriaToken tok = scanIdentifier(&L);
    onetest_assert(tok.type == TOK_VAR);
    onetest_assert(tok.start == 0);
    onetest_assert(tok.len == 3);

    skipWhitespace(&L);
    tok = scanIdentifier(&L);
    onetest_assert(tok.type == TOK_IDENTIFIER);
    onetest_assert(tok.start == 4);
    onetest_assert(tok.len == 4);

    return 0;
}

static inline int test_scanToken(void) {
    AriaLexer L = {0};
    ariaLexerInit(&L, "3 + 4");
    ariaTokenize(&L);

    AriaToken tok = scanToken(&L);
    onetest_assert(tok.valid == true);
    onetest_assert(tok.type == TOK_NUMBER);
    onetest_assert(tok.start == 0);
    onetest_assert(tok.len == 1);

    tok = scanToken(&L);
    onetest_assert(tok.type == TOK_PLUS);
    onetest_assert(tok.start == 2);
    onetest_assert(tok.len == 1);

    tok = scanToken(&L);
    onetest_assert(tok.type == TOK_NUMBER);
    onetest_assert(tok.start == 4);
    onetest_assert(tok.len == 1);

    tok = scanToken(&L);
    onetest_assert(tok.type == TOK_EOF);
    onetest_assert(tok.len == 0);

    // String
    L.source = "\"hello\"";
    L.pc = 0;

    tok = scanToken(&L);
    onetest_assert(tok.type == TOK_STRING);
    onetest_assert(tok.len == 7);

    // Identifier
    L.source = "variable";
    L.pc = 0;

    tok = scanToken(&L);
    onetest_assert(tok.type == TOK_IDENTIFIER);
    onetest_assert(tok.len == 8);

    return 0;
}

static inline int test_advance(void) {
    AriaLexer L = {0};
    ariaLexerInit(&L, "3 + 4");
    ariaTokenize(&L);

    advance(&L);
    const AriaToken* tok = &L.items[L.index];
    onetest_assert(tok->type == TOK_PLUS);

    return 0;
}

static inline int test_check(void) {
    AriaLexer L = {0};
    ariaLexerInit(&L, "3 + 4");
    ariaTokenize(&L);

    onetest_assert(check(&L, TOK_NUMBER) == 1);
    onetest_assert(check(&L, TOK_PLUS) == 0);

    return 0;
}

static inline int test_match(void) {
    AriaLexer L = {0};
    ariaLexerInit(&L, "3 + 4");
    ariaTokenize(&L);

    onetest_assert(match(&L, TOK_NUMBER));
    onetest_assert(match(&L, TOK_PLUS));
    return 0;
}

static inline int test_getCurrTokenType(void) {
    AriaLexer L = {0};
    ariaLexerInit(&L, "func foo() { return 42; }");
    ariaTokenize(&L);

    onetest_assert(getCurrTokenType(&L) == TOK_FUNC);
    advance(&L);
    onetest_assert(getCurrTokenType(&L) == TOK_IDENTIFIER);
    return 0;
}

static inline int test_getTokenNumber(void) {
    AriaLexer L = {0};
    ariaLexerInit(&L, "3 + 4");
    ariaTokenize(&L);

    int tok_num = getTokenNumber(&L, L.index);
    onetest_assert(tok_num == 3);
    return 0;
}

static inline int test_getTokenChar(void) {
    AriaLexer L = {0};
    ariaLexerInit(&L, "Hello");
    ariaTokenize(&L);

    char tok_char = getTokenChar(&L, L.index);
    onetest_assert(tok_char == 'H');
    return 0;
}

static inline int test_getTokenString(void) {
    AriaLexer L = {0};
    ariaLexerInit(&L, "\"foo\"");
    ariaTokenize(&L);

    char* str = getTokenString(&L, 0);
    onetest_assert(strcmp(str, "foo"));

    return 0;
}
