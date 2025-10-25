#ifndef ARIA_EXECUTOR_H
#define ARIA_EXECUTOR_H

#include "aria_bytecode.h"
#include "aria_stack.h"

int executeInstruction(Stack* global, Stack* local, Aria_Bytecode* inst, Aria_Buffer* idents);
int executeFunction(Stack* stack, Aria_Chunk* func, Aria_Buffer* idents);
Stack* ariaExecute(Aria_Module* mod);

#endif // ARIA_EXECUTOR_H
