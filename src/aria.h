#ifndef ARIA_H
#define ARIA_H

#define ARIA_DEBUG 1
#include "aria_lexer.h"
#include "aria_parser.h"

int aria_interpret(const char* name, const char* src);

#endif // ARIA_H
