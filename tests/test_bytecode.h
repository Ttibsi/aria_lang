#include <stdlib.h>

#include "onetest.h"
#include "aria_bytecode.h"
#include "aria_parser.h"
#include "aria_stack.h"

static int test_handleOperation(void) {
    Bytecode* bc = malloc(sizeof(Bytecode));
    Stack* stack = createStack(8);
    Expression* expr = malloc(sizeof(Expression));
    Expression* lhs =  malloc(sizeof(Expression));
    Expression* rhs = malloc(sizeof(Expression));

    lhs->type = Atom;
    lhs->c = 5;
    rhs->type = Atom;
    rhs->c = 2;

    expr->type = Operation;
    expr->op.ch = '+';
    expr->op.lhs = lhs;
    expr->op.rhs = rhs;

    bc = handleOperation(bc, stack, expr);
    onetest_assert_eq(bc->inst, INST_ADD);

    freeBytecode(bc);
    freeStack(stack);
    free(expr);
    free(lhs);
    free(rhs);

    return 0;
}

static int test_handleAtom(void) { return 1; }
static int test_nextInst(void) { return 1; }
static int test_bytecodeGeneration(void) { return 1; }
