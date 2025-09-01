#include <stddef.h>

#include "aria_executor.h"

Stack* ariaExecute(Bytecode* bc) {
    Stack* stack = createStack(1024);

    do {
        switch (bc->inst) {
            case INST_LOAD_CONST:
                stackPush(stack, bc->value);
                break;
            case INST_ADD: {
                    int b = stackPop(stack);
                    int a = stackPop(stack);
                    stackPush(stack, a + b);
                } break;
            case INST_SUB: {
                    int b = stackPop(stack);
                    int a = stackPop(stack);
                    stackPush(stack, a - b);
                } break;
            case INST_MUL: {
                    int b = stackPop(stack);
                    int a = stackPop(stack);
                    stackPush(stack, a*b);
                } break;
            case INST_DIV: {
                    int b = stackPop(stack);
                    int a = stackPop(stack);
                    stackPush(stack, a/b);
                } break;
            case INST_NULL:
                break;
        }

        bc = bc->next;
    } while (bc != NULL);

    return stack;
}
