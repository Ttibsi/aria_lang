#ifndef ARIA_EXECUTOR_H
#define ARIA_EXECUTOR_H

#include "aria_bytecode.h"
#include "aria_stack.h"

typedef struct {
    bool has_value;
    int value;
} OptionalInt;

OptionalInt executeInst(Aria_Bytecode* bc, Stack* stack, Aria_Chunk* c, Chunk_map_t* chunks);
[[nodiscard]] OptionalInt executeChunk(Aria_Chunk* c, Chunk_map_t* map);
[[nodiscard]] int ariaExecute(Aria_Module mod);

#endif  // ARIA_EXECUTOR_H
