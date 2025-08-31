#ifndef ARIA_STACK_H
#define ARIA_STACK_H

// source: https://www.digitalocean.com/community/tutorials/stack-in-c#how-to-implement-a-stack-in-c-with-code-examples

typedef struct {
    int top;
    int capacity;
    int* array;
} Stack;

Stack* createStack(int capacity);
int isFull(Stack* stack);
int isEmpty(Stack* stack);
void stackPush(Stack* stack, int item);
int stackPop(Stack* stack);
int stackPeek(Stack* stack);
void freeStack(Stack* stack);

#endif // ARIA_STACK_H
