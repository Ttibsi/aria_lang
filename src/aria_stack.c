#include "aria_stack.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

// Function to create a stack of given capacity.
Stack* createStack(int capacity) {
    Stack* stack = (Stack*)malloc(sizeof(Stack));
    if (!stack) {
        return NULL;
    }
    stack->capacity = capacity;
    stack->top = -1;

    stack->array = (int*)malloc(stack->capacity * sizeof(int));
    if (!stack->array) {
        free(stack);
        return NULL;
    }
    return stack;
}

int isFull(Stack* stack) {
    return stack->top == stack->capacity - 1;
}

int isEmpty(Stack* stack) {
    return stack->top == -1;
}

void stackPush(Stack* stack, int item) {
    if (isFull(stack)) {
        printf("Stack Overflow\n");
        return;
    }
    stack->array[++stack->top] = item;
}

int stackPop(Stack* stack) {
    if (isEmpty(stack)) {
        printf("Stack Underflow\n");
        return INT_MIN;
    }
    return stack->array[stack->top--];
}

int stackPeek(Stack* stack) {
    if (isEmpty(stack)) {
        printf("Stack is empty\n");
        return INT_MIN;
    }
    return stack->array[stack->top];
}

void freeStack(Stack* stack) {
    if (stack) {
        free(stack->array);
        free(stack);
    }
}
