#include "onetest.h"
#include "aria_lexer.h"
#include <stdbool.h>

static int test_peek(void) {
    Aria_Lexer l = (Aria_Lexer){
        .source = "3 + 4",
        .pc = 0,
        .current_token =  (Aria_Token){
            .valid = true,
            .type = TOK_NUMBER,
            .start = 0,
            .len = 1
        }
    };

    char actual = peek(&l);
    onetest_assert_eq(actual, '3');
    return 0;
}

static int test_peekNext(void) {
    Aria_Lexer l = (Aria_Lexer){
        .source = "3 + 4",
        .pc = 0,
        .current_token =  (Aria_Token){
            .valid = true,
            .type = TOK_NUMBER,
            .start = 0,
            .len = 1
        }
    };

    char actual = peekNext(&l);
    onetest_assert_eq(actual, ' ');
    return 0;
}

static int test_advanceChar(void) {
    Aria_Lexer l = (Aria_Lexer){
        .source = "3 + 4",
        .pc = 0,
        .current_token =  (Aria_Token){
            .valid = true,
            .type = TOK_NUMBER,
            .start = 0,
            .len = 1
        }
    };

    char actual = advanceChar(&l);
    onetest_assert_eq(actual, ' ');
    actual = advanceChar(&l);
    onetest_assert_eq(actual, '*');
    return 0;
}

static int test_skipWhitespace(void) {
    Aria_Lexer l = (Aria_Lexer){
        .source = "3 + 4",
        .pc = 0,
        .current_token =  (Aria_Token){
            .valid = true,
            .type = TOK_NUMBER,
            .start = 0,
            .len = 1
        }
    };

    skipWhitespace(&l);
    onetest_assert_eq(peek(&l), '*');
    return 0;
}

static int test_makeToken(void) {
    Aria_Lexer l = (Aria_Lexer){
        .source = "3 + 4",
        .pc = 0,
        .current_token =  (Aria_Token){
            .valid = true,
            .type = TOK_NUMBER,
            .start = 0,
            .len = 1
        }
    };

    Aria_Token tkn = makeToken(&l, TOK_STAR, 2, 1);
    onetest_assert_eq(tkn.valid, true);
    onetest_assert_eq(tkn.type, TOK_STAR);
    onetest_assert_eq(tkn.start, 1);
    onetest_assert_eq(tkn.len, 1);

    return 0;
}

static int test_scanEqualVariant(void) { return 1; }
static int test_scanStringLiteral(void) { return 1; }
static int test_scanNumber(void) { return 1; }
static int test_scanIdentifier(void) { return 1; }
static int test_scanToken(void) { return 1; }

static int test_advance(void) {
    Aria_Lexer l = (Aria_Lexer){
        .source = "3 + 4",
        .pc = 0,
        .current_token =  (Aria_Token){
            .valid = true,
            .type = TOK_NUMBER,
            .start = 0,
            .len = 1
        }
    };

    advance(&l);
    onetest_assert_eq(l.current_token.type, TOK_STAR);

    return 0;
}

static int test_check(void) {
    Aria_Lexer l = (Aria_Lexer){
        .source = "3 + 4",
        .pc = 0,
        .current_token =  (Aria_Token){
            .valid = true,
            .type = TOK_NUMBER,
            .start = 0,
            .len = 1
        }
    };

    onetest_assert_eq(check(&l, TOK_NUMBER), 1);
    onetest_assert_eq(check(&l, TOK_STAR), 0);

    return 0;
}

static int test_match(void) {
    Aria_Lexer l = (Aria_Lexer){
        .source = "3 + 4",
        .pc = 0,
        .current_token =  (Aria_Token){
            .valid = true,
            .type = TOK_NUMBER,
            .start = 0,
            .len = 1
        }
    };

    onetest_assert_eq(match(&l, TOK_NUMBER), 1);
    onetest_assert_eq(match(&l, TOK_STAR), 1);
    return 0;
}

static int test_getTokenNumber(void) { return 1; }
static int test_getTokenChar(void) { return 1; }
