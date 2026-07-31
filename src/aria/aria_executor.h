#ifndef ARIA_EXECUTOR_H
#define ARIA_EXECUTOR_H

#include "aria_bytecode.h"

typedef struct {
    bool has_value;
    int value;
} OptionalInt;

[[nodiscard]] OptionalInt executeChunk(Aria_Chunk* c, Chunk_map_t* map);
[[nodiscard]] int ariaExecute(Aria_Module mod);

#endif  // ARIA_EXECUTOR_H
