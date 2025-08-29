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
    // We're at the end of the chain here
    onetest_assert_true(bc->inst == -1);
    onetest_assert_true(bc->prev != NULL);
    onetest_assert_true(bc->prev->inst == INST_ADD);

    freeBytecode(bc);
    freeStack(stack);
    free(expr);
    free(lhs);
    free(rhs);

    return 0;
}

static int test_handleAtom(void) {
    Bytecode* bc = malloc(sizeof(Bytecode));
    Stack* stack = createStack(1);
    Expression* expr = malloc(sizeof(Expression));
    expr->type = Atom;
    expr->c = 5;

    bc = handleAtom(bc, stack, expr);
    onetest_assert_true(stackPeek(stack) == 5);
    return 0;
}

static int test_nextInst(void) {
    Bytecode* bc = malloc(sizeof(Bytecode));
    bc->inst = INST_ADD;

    bc = nextInst(bc, INST_LOAD_CONST);

    onetest_assert_true(bc->prev->inst == INST_LOAD_CONST);
    onetest_assert_true(bc->next == NULL);
    return 0;
}

static int test_bytecodeGeneration(void) {
    Stack* stack = createStack(8);
    Expression* lhs = malloc(sizeof(Expression));
    Expression* rhs = malloc(sizeof(Expression));
    Expression* expr = malloc(sizeof(Expression));

    // Test 1: Simple atom expression
    expr->type = Atom;
    expr->c = 42;

    Bytecode* bc1 = bytecodeGeneration(stack, *expr);
    onetest_assert_true(bc1->inst == INST_LOAD_CONST);
    onetest_assert_true(stackPeek(stack) == 42);
    onetest_assert_true(bc1->prev == NULL); // Should be root
    freeBytecode(bc1);

    // Clear stack for next test
    while (!isEmpty(stack)) {
        stackPop(stack);
    }

    // Test 2: Simple addition operation (5 + 3)
    lhs->type = Atom;
    lhs->c = 5;
    rhs->type = Atom;
    rhs->c = 3;

    expr->type = Operation;
    expr->op.ch = '+';
    expr->op.lhs = lhs;
    expr->op.rhs = rhs;

    Bytecode* bc2 = bytecodeGeneration(stack, *expr);

    // Should be at root, first instruction should be LOAD_CONST for lhs
    onetest_assert_true(bc2->inst == INST_LOAD_CONST);
    onetest_assert_true(bc2->prev == NULL); // Root node

    // Navigate through the bytecode chain
    Bytecode* current = bc2;
    onetest_assert_true(current->inst == INST_LOAD_CONST); // Load first operand
    current = current->next;
    onetest_assert_true(current->inst == INST_LOAD_CONST); // Load second operand
    current = current->next;
    onetest_assert_true(current->inst == INST_ADD); // Add operation

    // Check stack has both constants
    onetest_assert_true(!isEmpty(stack));

    freeBytecode(bc2);

    // Clear stack for next test
    while (!isEmpty(stack)) {
        stackPop(stack);
    }

    // Test 3: Simple multiplication operation (7 * 2)
    lhs->c = 7;
    rhs->c = 2;
    expr->op.ch = '*';

    Bytecode* bc3 = bytecodeGeneration(stack, *expr);

    // Navigate through bytecode to find multiplication
    current = bc3;
    while (current->next != NULL) {
        current = current->next;
    }
    onetest_assert_true(current->inst == -1);

    freeBytecode(bc3);
    freeStack(stack);
    free(lhs);
    free(rhs);
    free(expr);

    return 0;
}
