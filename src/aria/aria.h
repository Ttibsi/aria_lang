#ifndef ARIA_H
#define ARIA_H

#include "aria_buffer.h"
#include "aria_lexer.h"


[[nodiscard]] char* ariaVersion();
[[nodiscard]] int ariaInterpret(char* text, const bool debug_mode);

#endif // ARIA_H
