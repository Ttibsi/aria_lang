#ifndef ARIA_EXECUTOR_H
#define ARIA_EXECUTOR_H

#include "aria_bytecode.h"
#include "aria_stack.h"

Stack* ariaExecute(Bytecode* bc);

#endif // ARIA_EXECUTOR_H
