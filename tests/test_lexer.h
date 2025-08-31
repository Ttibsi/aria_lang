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
    onetest_assert_eq(actual, '3');
    actual = advanceChar(&l);
    onetest_assert_eq(actual, ' ');
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

    advance(&l);
    skipWhitespace(&l);
    onetest_assert_eq(peek(&l), '+');
    return 0;
}

static int test_makeToken(void) {
    Aria_Token tkn = makeToken(TOK_PLUS, 2, 1);
    onetest_assert_eq(tkn.valid, true);
    onetest_assert_eq(tkn.type, TOK_PLUS);
    onetest_assert_eq(tkn.start, 2);
    onetest_assert_eq(tkn.len, 1);

    return 0;
}

static int test_scanEqualVariant(void) {
    Aria_Lexer l = (Aria_Lexer){
        .source = "!=",
        .pc = 1,
        .current_token =  (Aria_Token){
            .valid = true,
            .type = TOK_NUMBER,
            .start = 0,
            .len = 1
        }
    };

    Aria_Token tok = scanEqualVariant(&l, TOK_BANG, TOK_BANG_EQUAL);
    onetest_assert_true(tok.type == TOK_BANG_EQUAL);

    l.source = "<";
    tok = scanEqualVariant(&l, TOK_LESS, TOK_LESS_EQUAL);
    onetest_assert_true(tok.type == TOK_LESS);

    return 0;
}

static int test_scanStringLiteral(void) {
        // Test 1: Normal string literal
    Aria_Lexer l1 = (Aria_Lexer){
        .source = "\"hello\"",
        .pc = 1,  // positioned after opening quote
        .current_token = (Aria_Token){
            .valid = true,
            .type = TOK_STRING,
            .start = 0,
            .len = 1
        }
    };

    Aria_Token tok1 = scanStringLiteral(&l1);
    onetest_assert_true(tok1.valid == true);
    onetest_assert_true(tok1.type == TOK_STRING);
    onetest_assert_true(tok1.start == 0);
    onetest_assert_true(tok1.len == 7); // "hello" = 5 chars + 2 quotes
    onetest_assert_true(l1.pc == 7); // should be positioned after closing quote

    // Test 2: Empty string
    Aria_Lexer l2 = (Aria_Lexer){
        .source = "\"\"",
        .pc = 1,  // positioned after opening quote
        .current_token = (Aria_Token){
            .valid = true,
            .type = TOK_STRING,
            .start = 0,
            .len = 1
        }
    };

    Aria_Token tok2 = scanStringLiteral(&l2);
    onetest_assert_true(tok2.valid == true);
    onetest_assert_true(tok2.type == TOK_STRING);
    onetest_assert_true(tok2.start == 0);
    onetest_assert_true(tok2.len == 2); // just the two quotes
    onetest_assert_true(l2.pc == 2); // should be positioned after closing quote

    // Test 3: String with spaces and special characters
    Aria_Lexer l3 = (Aria_Lexer){
        .source = "\"hello world 123!@#\"",
        .pc = 1,  // positioned after opening quote
        .current_token = (Aria_Token){
            .valid = true,
            .type = TOK_STRING,
            .start = 0,
            .len = 1
        }
    };

    Aria_Token tok3 = scanStringLiteral(&l3);
    onetest_assert_true(tok3.valid == true);
    onetest_assert_true(tok3.type == TOK_STRING);
    onetest_assert_true(tok3.start == 0);
    onetest_assert_true(tok3.len == 20); // 18 chars + 2 quotes
    onetest_assert_true(l3.pc == 20); // should be positioned after closing quote

    // Test 4: Unterminated string (error case)
    Aria_Lexer l4 = (Aria_Lexer){
        .source = "\"unterminated",
        .pc = 1,  // positioned after opening quote
        .current_token = (Aria_Token){
            .valid = true,
            .type = TOK_STRING,
            .start = 0,
            .len = 1
        }
    };

    Aria_Token tok4 = scanStringLiteral(&l4);
    onetest_assert_true(tok4.valid == true);
    onetest_assert_true(tok4.type == TOK_EOF); // function returns EOF for error case
    onetest_assert_true(tok4.start == 0);
    onetest_assert_true(tok4.len == 0);

    return 0;
}

static int test_scanNumber(void) {
    Aria_Lexer l = (Aria_Lexer){
        .source = "123 + 45",
        .pc = 0,
        .current_token =  (Aria_Token){
            .valid = true,
            .type = TOK_NUMBER,
            .start = 0,
            .len = 1
        }
    };

    Aria_Token tok = scanNumber(&l);
    onetest_assert_true(tok.type == TOK_NUMBER);
    onetest_assert_true(tok.len == 3);

    return 0;
}

static int test_scanIdentifier(void) {
    Aria_Lexer l = (Aria_Lexer){
        .source = "var temp",
        .pc = 0,
        .current_token =  (Aria_Token){
            .valid = true,
            .type = TOK_NUMBER,
            .start = 0,
            .len = 1
        }
    };

    Aria_Token tok = scanIdentifier(&l);
    onetest_assert_true(tok.type == TOK_VAR);
    onetest_assert_true(tok.start == 0);
    onetest_assert_true(tok.len == 3);

    skipWhitespace(&l);
    tok = scanIdentifier(&l);
    onetest_assert_true(tok.type == TOK_IDENTIFIER);
    onetest_assert_true(tok.start == 4);
    onetest_assert_true(tok.len == 4);

    return 0;
}

static int test_scanToken(void) {
    Aria_Lexer l = (Aria_Lexer){
        .source = "3 + 4",
        .pc = 0,
        .current_token = (Aria_Token){
            .valid = true,
            .type = TOK_NUMBER,
            .start = 0,
            .len = 1
        }
    };

    // Test 1: Number token
    Aria_Token tok = scanToken(&l);
    onetest_assert_true(tok.valid == true);
    onetest_assert_true(tok.type == TOK_NUMBER);
    onetest_assert_true(tok.start == 0);
    onetest_assert_true(tok.len == 1);

    // Test 2: Plus token (after whitespace)
    tok = scanToken(&l);
    onetest_assert_true(tok.type == TOK_PLUS);
    onetest_assert_true(tok.start == 2);
    onetest_assert_true(tok.len == 1);

    // Test 3: Another number token
    tok = scanToken(&l);
    onetest_assert_true(tok.type == TOK_NUMBER);
    onetest_assert_true(tok.start == 4);
    onetest_assert_true(tok.len == 1);

    // Test 4: EOF at end
    tok = scanToken(&l);
    onetest_assert_true(tok.type == TOK_EOF);
    onetest_assert_true(tok.len == 0);

    // Test 5: Reset lexer for string test
    l.source = "\"hello\"";
    l.pc = 0;

    tok = scanToken(&l);
    onetest_assert_true(tok.type == TOK_STRING);
    onetest_assert_true(tok.len == 7);

    // Test 6: Reset lexer for identifier test
    l.source = "variable";
    l.pc = 0;

    tok = scanToken(&l);
    onetest_assert_true(tok.type == TOK_IDENTIFIER);
    onetest_assert_true(tok.len == 8);

    return 0;
}

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
    onetest_assert_eq(l.current_token.type, TOK_NUMBER);

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
    onetest_assert_eq(check(&l, TOK_PLUS), 0);

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

    onetest_assert_true(match(&l, TOK_NUMBER) == 1);
    advance(&l);
    onetest_assert_true(match(&l, TOK_PLUS) == 1);
    return 0;
}

static int test_getTokenNumber(void) {
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

    onetest_assert_true(getTokenNumber(&l, l.current_token) == 3);
    return 0;
}

static int test_getTokenChar(void) {
    Aria_Lexer l = (Aria_Lexer){
        .source = "Hello",
        .pc = 0,
        .current_token =  (Aria_Token){
            .valid = true,
            .type = TOK_STRING,
            .start = 0,
            .len = 1
        }
    };

    onetest_assert_true(getTokenChar(&l, l.current_token) == 'H');
    return 0;
}
