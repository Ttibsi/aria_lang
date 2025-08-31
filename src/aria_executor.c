#include <stddef.h>

#include "aria_executor.h"

Stack* ariaExecute(Bytecode* bc) {
    Stack* stack = createStack(1024);

    do {
        int a;
        int b;

        switch (bc->inst) {
            case INST_LOAD_CONST:
                stackPush(stack, bc->value);
                break;
            case INST_ADD:
                b = stackPop(stack);
                a = stackPop(stack);
                stackPush(stack, a + b);
                break;
            case INST_MUL:
                b = stackPop(stack);
                a = stackPop(stack);
                stackPush(stack, a*b);
                break;
        }

        bc = bc->next;
    } while (bc != NULL);

    return stack;
}
