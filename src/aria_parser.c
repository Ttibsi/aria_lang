#include "aria_lexer.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "aria_parser.h"

typedef struct {
    float lhs;
    float rhs;
} BP;

BP infix_binding_power(char op) {
    switch (op) {
        case '+': case '-': return (BP){.lhs = 1.0, .rhs = 1.1};
        case '*': case '/': return (BP){.lhs = 2.0, .rhs = 2.1};
    }

    return (BP){.lhs = 0.0, .rhs = 0.0};
}

void advance_state(ParserState* state, Aria_Lexer* l) {
    state->prev = state->curr;
    state->curr = state->next;
    state->next = malloc(sizeof(Aria_Token));
    *state->next = scanToken(l);
}

void state_assert(ParserState* state, int lineno, int count, ...) {
    va_list tok_types;
    TokenType arg;

    va_start(tok_types, count);
    for (int i = 0; i < count; i ++) {
        arg = va_arg(tok_types, TokenType);
        if (state->curr->type == arg) {
            va_end(tok_types);
            return;
        }
    }
    va_end(tok_types);

    fprintf(stderr, "Unexpected token %d at line %d, expected one of: ", state->curr->type, lineno);
    va_start(tok_types, count);
    for (int i = 0; i < count; i ++) {
        arg = va_arg(tok_types, TokenType);
        fprintf(stderr, "%d, ", arg);
    }
    va_end(tok_types);
    fputc('\n', stderr);

    assert(0);
}

Expression parse_expression(ParserState* state, Aria_Lexer* l, float min_bp) {
    state_assert(state, __LINE__, 2, TOK_NUMBER, TOK_LEFT_PAREN);
    Expression lhs = {0};

    switch (state->curr->type) {
        case TOK_NUMBER:
            lhs = (Expression){
                .type = Atom,
                .c = get_token_number(l, *state->curr)
            };
            break;

        case TOK_LEFT_PAREN:
            advance_state(state, l);
            lhs = parse_expression(state, l, 0);
            state_assert(state, __LINE__, 2, TOK_RIGHT_PAREN, -1);
            break;

        default:
            assert(0);
    };

    // EOF
    if (state->next->type == TOK_EOF) { return lhs; }

    // end of expression
    if (state->next->type == TOK_RIGHT_PAREN) { return lhs; }

    while (true) {
        if (state->next == NULL) { break; }
        advance_state(state, l);
        if (state->next->type == TOK_EOF) { break; }
        state_assert(
            state, __LINE__, 5,
            TOK_MINUS, TOK_PLUS, TOK_SLASH, TOK_STAR, TOK_RIGHT_PAREN
        );

        if (state->curr->type == TOK_RIGHT_PAREN) { break; }

        char op = get_token_char(l, *state->curr);
        BP bp = infix_binding_power(op);
        if (bp.lhs < min_bp) { break; }

        // rhs
        advance_state(state, l);
        state_assert(state, __LINE__, 2, TOK_LEFT_PAREN, TOK_NUMBER);

        Expression rhs = parse_expression(state, l, bp.rhs);

        Expression* lhs_ptr = malloc(sizeof(Expression));
        Expression* rhs_ptr = malloc(sizeof(Expression));
        *lhs_ptr = lhs;  // Copy current lhs value
        *rhs_ptr = rhs;  // Copy rhs value

        lhs = (Expression){
            .type = Operation,
            .op = {.ch = op, .lhs = lhs_ptr, .rhs = rhs_ptr}
        };
    }

    return lhs;
}

void print_exprs(Expression expr) {
    if (expr.type == Operation) {
        printf("(%c ", expr.op.ch);
        print_exprs(*expr.op.lhs);
        print_exprs(*expr.op.rhs);
        printf(")");
    } else {
        // Atom
        printf(" %i ", expr.c);
    }
}
