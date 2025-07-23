#ifndef ARIA_H
#define ARIA_H

#define ARIA_DEBUG 1 
#include "aria_lexer.h"
#include "aria_parser.h"

int aria_interpret(Aria_VM* vm, const char* name, const char* src);
Aria_VM aria_vm_init(void);
void aria_vm_destroy(Aria_VM* vm);

#endif // ARIA_H
