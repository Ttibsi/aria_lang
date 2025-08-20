#ifndef ARIA_PARSER_H
#define ARIA_PARSER_H

#include "aria_lexer.h"

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

Expression parse_expression(ParserState* state, Aria_Lexer* l, float min_bp);
void print_exprs(Expression expr);
void advance_state(ParserState* state, Aria_Lexer* l);
#endif // ARIA_PARSER_H
