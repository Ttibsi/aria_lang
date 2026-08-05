#ifndef ARIA_STACK_H
#define ARIA_STACK_H

#include <assert.h>
#include <stddef.h>

#include "nob.h"

typedef struct {
    int* items;
    size_t count;
    size_t capacity;
} Stack;

[[nodiscard]] int stackTop(Stack* stack);
void stackPush(Stack* stack, int value);
[[nodiscard]] int stackPop(Stack* stack);
[[nodiscard]] int* stackGet(Stack* stack, size_t idx);
[[nodiscard]] int stackSize(Stack* stack);
void stackRewind(Stack* stack, size_t savePoint);
[[nodiscard]] size_t stackSave(Stack* stack);

#ifdef ARIA_STACK_IMPL

int stackTop(Stack* stack) { return *(stack->items + stack->count); }

void stackPush(Stack* stack, int value) { nob_da_append(stack, value); }

int stackPop(Stack* stack) {
    stack->count--;
    return *(stack->items + stack->count);
}

int* stackGet(Stack* stack, size_t idx) { return stack->items + idx; }

int stackSize(Stack* stack) { return stack->count; }

void stackRewind(Stack* stack, size_t savePoint) {
    assert(savePoint < stack->count);
    stack->count = savePoint;
}

size_t stackSave(Stack* stack) { return stack->count; }

#endif  // ARIA_STACK_IMPL
#endif  // ARIA_STACK_H
