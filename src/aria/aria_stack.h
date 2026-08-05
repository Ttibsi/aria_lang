#ifndef ARIA_STACK_H
#define ARIA_STACK_H

#include <assert.h>
#include <stddef.h>

#include "nob.h"

#if 0
// source:
// https://www.digitalocean.com/community/tutorials/stack-in-c#how-to-implement-a-stack-in-c-with-code-examples

typedef struct {
    int top;
    int capacity;
    int* array;
} Stack;

[[maybe_unused]] Stack* createStack(const int capacity);
[[maybe_unused]] int isFull(const Stack* stack);
[[maybe_unused]] int isEmpty(const Stack* stack);
[[maybe_unused]] void stackPush(Stack* stack, int item);
[[maybe_unused]] int stackPop(Stack* stack);
[[maybe_unused]] int stackPeek(const Stack* stack);
[[maybe_unused]] void freeStack(Stack* stack);

#ifdef ARIA_STACK_IMPL

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

// Function to create a stack of given capacity.
[[maybe_unused]] Stack* createStack(const int capacity) {
    Stack* stack = (Stack*)malloc(sizeof(Stack));

    stack->capacity = capacity;
    stack->top = -1;
    stack->array = (int*)malloc(stack->capacity * sizeof(int));

    return stack;
}

[[maybe_unused]] int isFull(const Stack* stack) { return stack->top == stack->capacity - 1; }

[[maybe_unused]] int isEmpty(const Stack* stack) { return stack->top == -1; }

[[maybe_unused]] void stackPush(Stack* stack, int item) {
    if (isFull(stack)) {
        printf("Stack Overflow\n");
        return;
    }
    stack->array[++stack->top] = item;
}

[[maybe_unused]] int stackPop(Stack* stack) {
    if (isEmpty(stack)) {
        printf("Stack Underflow\n");
        return INT_MIN;
    }
    return stack->array[stack->top--];
}

[[maybe_unused]] int stackPeek(const Stack* stack) {
    if (isEmpty(stack)) {
        printf("Stack is empty\n");
        return INT_MIN;
    }
    return stack->array[stack->top];
}

[[maybe_unused]] void freeStack(Stack* stack) {
    if (stack != NULL) {
        free(stack->array);
        free(stack);
    }
}

#endif  // ARIA_STACK_IMPL
#else

typedef struct {
    int* items;
    size_t count;
    size_t capacity;
} Stack;

inline int stackTop(Stack* stack) { return *(stack->items + stack->count); }

inline void stackPush(Stack* stack, int value) { nob_da_append(stack, value); }

inline int stackPop(Stack* stack) {
    stack->count--;
    return *(stack->items + stack->count);
}

inline int* stackGet(Stack* stack, size_t idx) { return stack->items + idx; }

inline int stackSize(Stack* stack) { return stack->count; }

inline void stackRewind(Stack* stack, size_t savePoint) {
    assert(savePoint < stack->count);
    stack->count = savePoint;
}

inline size_t stackSave(Stack* stack) { return stack->count; }
#endif

#endif  // ARIA_STACK_H
