#ifndef ARIA_PARSER_H
#define ARIA_PARSER_H

#include "aria_lexer.h"

typedef struct {
    float lhs;
    float rhs;
} BP;

typedef enum {
    Atom,
    Operation
} ExprType;
struct Expr;

typedef struct Expr {
    ExprType type;
    union {
        int c;

        struct {
            char ch;
            struct Expr* lhs;
            struct Expr* rhs;
        } op;
    };
} Expression;

typedef struct {
    Aria_Token* prev;
    Aria_Token* curr;
    Aria_Token* next;
} ParserState;

BP infixBindingPower(char op);
void advanceState(ParserState* state, Aria_Lexer* l);
Expression parseExpression(ParserState* state, Aria_Lexer* l, float min_bp);
void printExprs(Expression expr);
void advanceState(ParserState* state, Aria_Lexer* l);
#endif // ARIA_PARSER_H
