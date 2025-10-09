#ifndef ARIA_EXECUTOR_H
#define ARIA_EXECUTOR_H

#include "aria_bytecode.h"
#include "aria_stack.h"
#include "aria_bytecode.h"

void executeInstruction(Stack* global, Stack* local, Aria_Bytecode* inst);
int executeFunction(Stack* stack, Aria_Chunk* func);
Stack* ariaExecute(Aria_Module* mod);

#endif // ARIA_EXECUTOR_H
