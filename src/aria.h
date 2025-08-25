#ifndef ARIA_H
#define ARIA_H

extern int aria_debug_mode;
#include "aria_lexer.h"
#include "aria_parser.h"
#include "aria_bytecode.h"

int aria_interpret(const char* name, const char* src);

#endif // ARIA_H
