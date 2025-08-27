#include "onetest.h"
#include "aria_parser.h"

static int test_infixBindingPower(void) {
    BP ret = infixBindingPower('+');
    onetest_assert_eq(ret.lhs, 1.0);

    BP ret2 = infixBindingPower('/');
    onetest_assert_eq(ret2.rhs, 2.1f);
    return 0;
}

static int test_advanceState(void) {
    ParserState* state = malloc(sizeof(ParserState));
    state->prev = NULL;
    state->curr = NULL;
    state->next = malloc(sizeof(Aria_Token));
    *state->next = (Aria_Token){
        .valid = false,
        .type = TOK_ERROR,
        .start = 0,
        .len = 0
    };

    Aria_Lexer l = (Aria_Lexer){
        .source = "Hello",
        .pc = 0,
        .current_token = (Aria_Token){
            .valid = true,
            .type = TOK_IDENTIFIER,
            .start = 0,
            .len = 3
        },
    };

    advanceState(state, &l);

    free(state->next);
    free(state);

    return 0;
}

static int test_parseExpression(void) {
    // Test 1: Simple number parsing - minimal case
    {
        Aria_Lexer l = (Aria_Lexer){
            .source = "42",
            .pc = 0,
            .current_token = (Aria_Token){
                .valid = true,
                .type = TOK_NUMBER,
                .start = 0,
                .len = 2
            }
        };

        ParserState* state = malloc(sizeof(ParserState));
        state->prev = NULL;
        state->curr = malloc(sizeof(Aria_Token));
        state->next = malloc(sizeof(Aria_Token));

        // Set current token as number
        *state->curr = (Aria_Token){
            .valid = true,
            .type = TOK_NUMBER,
            .start = 0,
            .len = 2
        };

        // Set next token as EOF to end parsing
        *state->next = (Aria_Token){
            .valid = true,
            .type = TOK_EOF,
            .start = 2,
            .len = 0
        };

        Expression expr = parseExpression(state, &l, 0);

        onetest_assert_eq(expr.type, Atom);

        free(state->curr);
        free(state->next);
        free(state);
    }

    // Test 2: Test with simple expression that should work
    // This test will depend on your lexer implementation working correctly
    {
        // We'll test a very simple case: just "5"
        Aria_Lexer l = (Aria_Lexer){
            .source = "5",
            .pc = 0,
            .current_token = (Aria_Token){
                .valid = true,
                .type = TOK_NUMBER,
                .start = 0,
                .len = 1
            }
        };

        ParserState* state = malloc(sizeof(ParserState));
        state->prev = NULL;
        state->curr = malloc(sizeof(Aria_Token));
        state->next = malloc(sizeof(Aria_Token));

        *state->curr = (Aria_Token){
            .valid = true,
            .type = TOK_NUMBER,
            .start = 0,
            .len = 1
        };

        *state->next = (Aria_Token){
            .valid = true,
            .type = TOK_EOF,
            .start = 1,
            .len = 0
        };

        Expression expr = parseExpression(state, &l, 0);

        // Just test that we got an atom back
        onetest_assert_eq(expr.type, Atom);

        free(state->curr);
        free(state->next);
        free(state);
    }

    return 0;
}
