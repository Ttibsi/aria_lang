#include "onetest.h"
#include "aria_lexer.h"
#include <stdbool.h>

static int test_peek(void) {
    Aria_Lexer l = (Aria_Lexer){
        .source = "3 + 4",
        .pc = 0,
        .tokens = bufferCreate(sizeof(Aria_Token), 8),
        .buf_index = 0
    };

    char actual = peek(&l);
    onetest_assert(actual == '3');
    return 0;
}

static int test_peekNext(void) {
    Aria_Lexer l = (Aria_Lexer){
        .source = "3 + 4",
        .pc = 0,
        .tokens = bufferCreate(sizeof(Aria_Token), 8),
        .buf_index = 0
    };

    char actual = peekNext(&l);
    onetest_assert(actual == ' ');
    return 0;
}

static int test_advanceChar(void) {
    Aria_Lexer l = (Aria_Lexer){
        .source = "3 + 4",
        .pc = 0,
        .tokens = bufferCreate(sizeof(Aria_Token), 8),
        .buf_index = 0
    };

    char actual = advanceChar(&l);
    onetest_assert(actual == '3');
    actual = advanceChar(&l);
    onetest_assert(actual == ' ');
    return 0;
}

static int test_skipWhitespace(void) {
    Aria_Lexer l = (Aria_Lexer){
        .source = "3 + 4",
        .pc = 0,
        .tokens = bufferCreate(sizeof(Aria_Token), 8),
        .buf_index = 0
    };

    advanceChar(&l);
    skipWhitespace(&l);
    onetest_assert(peek(&l) == '+');
    return 0;
}

static int test_makeToken(void) {
    Aria_Token tkn = makeToken(TOK_PLUS, 2, 1);
    onetest_assert(tkn.valid == true);
    onetest_assert(tkn.type == TOK_PLUS);
    onetest_assert(tkn.start == 2);
    onetest_assert(tkn.len == 1);

    return 0;
}

static int test_scanEqualVariant(void) {
    Aria_Lexer l = (Aria_Lexer){
        .source = "!=",
        .pc = 1,
        .tokens = bufferCreate(sizeof(Aria_Token), 8),
        .buf_index = 0
    };

    Aria_Token tok = scanEqualVariant(&l, TOK_BANG, TOK_BANG_EQUAL);
    onetest_assert(tok.type == TOK_BANG_EQUAL);

    l.source = "<";
    tok = scanEqualVariant(&l, TOK_LESS, TOK_LESS_EQUAL);
    onetest_assert(tok.type == TOK_LESS);

    return 0;
}

static int test_scanStringLiteral(void) {
        // Test 1: Normal string literal
    Aria_Lexer l1 = (Aria_Lexer){
        .source = "\"hello\"",
        .pc = 1,  // positioned after opening quote
        .tokens = bufferCreate(sizeof(Aria_Token), 8),
        .buf_index = 0
    };

    Aria_Token tok1 = scanStringLiteral(&l1);
    onetest_assert(tok1.valid == true);
    onetest_assert(tok1.type == TOK_STRING);
    onetest_assert(tok1.start == 0);
    onetest_assert(tok1.len == 7); // "hello" = 5 chars + 2 quotes
    onetest_assert(l1.pc == 7); // should be positioned after closing quote

    // Test 2: Empty string
    Aria_Lexer l2 = (Aria_Lexer){
        .source = "\"\"",
        .pc = 1,  // positioned after opening quote
        .tokens = bufferCreate(sizeof(Aria_Token), 8),
        .buf_index = 0
    };

    Aria_Token tok2 = scanStringLiteral(&l2);
    onetest_assert(tok2.valid == true);
    onetest_assert(tok2.type == TOK_STRING);
    onetest_assert(tok2.start == 0);
    onetest_assert(tok2.len == 2); // just the two quotes
    onetest_assert(l2.pc == 2); // should be positioned after closing quote

    // Test 3: String with spaces and special characters
    Aria_Lexer l3 = (Aria_Lexer){
        .source = "\"hello world 123!@#\"",
        .pc = 1,  // positioned after opening quote
        .tokens = bufferCreate(sizeof(Aria_Token), 8),
        .buf_index = 0
    };

    Aria_Token tok3 = scanStringLiteral(&l3);
    onetest_assert(tok3.valid == true);
    onetest_assert(tok3.type == TOK_STRING);
    onetest_assert(tok3.start == 0);
    onetest_assert(tok3.len == 20); // 18 chars + 2 quotes
    onetest_assert(l3.pc == 20); // should be positioned after closing quote

    // Test 4: Unterminated string (error case)
    Aria_Lexer l4 = (Aria_Lexer){
        .source = "\"unterminated",
        .pc = 1,  // positioned after opening quote
        .tokens = bufferCreate(sizeof(Aria_Token), 8),
        .buf_index = 0
    };

    Aria_Token tok4 = scanStringLiteral(&l4);
    onetest_assert(tok4.valid == true);
    onetest_assert(tok4.type == TOK_EOF); // function returns EOF for error case
    onetest_assert(tok4.start == 0);
    onetest_assert(tok4.len == 0);

    return 0;
}

static int test_scanNumber(void) {
    Aria_Lexer l = (Aria_Lexer){
        .source = "123 + 45",
        .pc = 0,
        .tokens = bufferCreate(sizeof(Aria_Token), 8),
        .buf_index = 0
    };

    Aria_Token tok = scanNumber(&l);
    onetest_assert(tok.type == TOK_NUMBER);
    onetest_assert(tok.len == 3);

    return 0;
}

static int test_scanIdentifier(void) {
    Aria_Lexer l = (Aria_Lexer){
        .source = "var temp",
        .pc = 0,
        .tokens = bufferCreate(sizeof(Aria_Token), 8),
        .buf_index = 0
    };

    Aria_Token tok = scanIdentifier(&l);
    onetest_assert(tok.type == TOK_VAR);
    onetest_assert(tok.start == 0);
    onetest_assert(tok.len == 3);

    skipWhitespace(&l);
    tok = scanIdentifier(&l);
    onetest_assert(tok.type == TOK_IDENTIFIER);
    onetest_assert(tok.start == 4);
    onetest_assert(tok.len == 4);

    return 0;
}

static int test_scanToken(void) {
    Aria_Lexer l = (Aria_Lexer){
        .source = "3 + 4",
        .pc = 0,
        .tokens = bufferCreate(sizeof(Aria_Token), 8),
        .buf_index = 0
    };

    // Test 1: Number token
    Aria_Token tok = scanToken(&l);
    onetest_assert(tok.valid == true);
    onetest_assert(tok.type == TOK_NUMBER);
    onetest_assert(tok.start == 0);
    onetest_assert(tok.len == 1);

    // Test 2: Plus token (after whitespace)
    tok = scanToken(&l);
    onetest_assert(tok.type == TOK_PLUS);
    onetest_assert(tok.start == 2);
    onetest_assert(tok.len == 1);

    // Test 3: Another number token
    tok = scanToken(&l);
    onetest_assert(tok.type == TOK_NUMBER);
    onetest_assert(tok.start == 4);
    onetest_assert(tok.len == 1);

    // Test 4: EOF at end
    tok = scanToken(&l);
    onetest_assert(tok.type == TOK_EOF);
    onetest_assert(tok.len == 0);

    // Test 5: Reset lexer for string test
    l.source = "\"hello\"";
    l.pc = 0;

    tok = scanToken(&l);
    onetest_assert(tok.type == TOK_STRING);
    onetest_assert(tok.len == 7);

    // Test 6: Reset lexer for identifier test
    l.source = "variable";
    l.pc = 0;

    tok = scanToken(&l);
    onetest_assert(tok.type == TOK_IDENTIFIER);
    onetest_assert(tok.len == 8);

    return 0;
}

static int test_advance(void) {
    Aria_Lexer l = (Aria_Lexer){
        .source = "3 + 4",
        .pc = 0,
        .tokens = bufferCreate(sizeof(Aria_Token), 8),
        .buf_index = 0
    };

    Aria_Token tok1 = (Aria_Token){true, TOK_NUMBER, 0, 1};
    bufferInsert(&l.tokens, &tok1);

    Aria_Token tok2 = (Aria_Token){true, TOK_PLUS, 2, 1};
    bufferInsert(&l.tokens, &tok2);

    Aria_Token tok3 = (Aria_Token){true, TOK_NUMBER, 4, 1};
    bufferInsert(&l.tokens, &tok3);

    advance(&l);
    Aria_Token* tok = (Aria_Token*)bufferGet(l.tokens, l.buf_index);
    onetest_assert(tok->type == TOK_PLUS);

    return 0;
}

static int test_check(void) {
    Aria_Lexer l = (Aria_Lexer){
        .source = "3 + 4",
        .pc = 0,
        .tokens = bufferCreate(sizeof(Aria_Token), 8),
        .buf_index = 0
    };

    Aria_Token tok1 = (Aria_Token){true, TOK_NUMBER, 0, 1};
    bufferInsert(&l.tokens, &tok1);

    Aria_Token tok2 = (Aria_Token){true, TOK_PLUS, 2, 1};
    bufferInsert(&l.tokens, &tok2);

    Aria_Token tok3 = (Aria_Token){true, TOK_NUMBER, 4, 1};
    bufferInsert(&l.tokens, &tok3);

    onetest_assert(check(&l, TOK_NUMBER) == 1);
    onetest_assert(check(&l, TOK_PLUS) == 0);

    return 0;
}

static int test_match(void) {
    Aria_Lexer l = (Aria_Lexer){
        .source = "3 + 4",
        .pc = 0,
        .tokens = bufferCreate(sizeof(Aria_Token), 8),
        .buf_index = 0
    };

    Aria_Token tok1 = (Aria_Token){true, TOK_NUMBER, 0, 1};
    bufferInsert(&l.tokens, &tok1);

    Aria_Token tok2 = (Aria_Token){true, TOK_PLUS, 2, 1};
    bufferInsert(&l.tokens, &tok2);

    Aria_Token tok3 = (Aria_Token){true, TOK_NUMBER, 4, 1};
    bufferInsert(&l.tokens, &tok3);

    onetest_assert(match(&l, TOK_NUMBER));
    onetest_assert(match(&l, TOK_PLUS));
    return 0;
}

static int test_getCurrToken_type(void) { return 1; }

static int test_getTokenNumber(void) {
    Aria_Lexer l = (Aria_Lexer){
        .source = "3 + 4",
        .pc = 0,
        .tokens = bufferCreate(sizeof(Aria_Token), 8),
        .buf_index = 0
    };

    Aria_Token tok1 = (Aria_Token){true, TOK_NUMBER, 0, 1};
    bufferInsert(&l.tokens, &tok1);

    Aria_Token tok2 = (Aria_Token){true, TOK_PLUS, 2, 1};
    bufferInsert(&l.tokens, &tok2);

    Aria_Token tok3 = (Aria_Token){true, TOK_NUMBER, 4, 1};
    bufferInsert(&l.tokens, &tok3);

    int tok_num = getTokenNumber(&l, *(Aria_Token*)bufferGet(l.tokens, l.buf_index));
    onetest_assert(tok_num == 3);
    return 0;
}

static int test_getTokenChar(void) {
    Aria_Lexer l = (Aria_Lexer){
        .source = "Hello",
        .pc = 0,
        .tokens = bufferCreate(sizeof(Aria_Token), 8),
        .buf_index = 0
    };

    Aria_Token tok1 = (Aria_Token){true, TOK_STRING, 0, 5};
    bufferInsert(&l.tokens, &tok1);

    char tok_char = getTokenChar(&l, *(Aria_Token*)bufferGet(l.tokens, l.buf_index));
    onetest_assert(tok_char == 'H');
    return 0;
}
