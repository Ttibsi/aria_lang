#ifndef ARIA_H
#define ARIA_H

#include <stdlib.h>

#include "aria_lexer.h"

typedef enum {
    OBJ_TYPE_NUM,
    OBJ_TYPE_BOOL,
    OBJ_TYPE_CHAR,
    OBJ_TYPE_STR,
    OBJ_TYPE_NIL,
} AriaType;

typedef struct {
    AriaType type;
    union {
        double i;
        bool b;
        char c;
        char* s;
    } as;
} AriaObj;

// For use with nob_da_append functions
// NOTE: nob_da_append and nob_da_reserve handle initialisation for us
typedef struct {
    AriaObj* items;
    size_t count;
    size_t capacity;
} AriaStack;

typedef struct _AriaVM {
    AriaStack global_stack;
    int argc;
    char** argv;
    bool debug_mode;

    AriaLexer lexer;

    void (*writeFn)(struct _AriaVM*);
} AriaVM;

typedef int (*ariaFuncPtr)(AriaVM*);
typedef struct {
    char* name;
    ariaFuncPtr f;
} AriaFunc;

typedef struct {
    char* name;

    // Array to iterate until we hit the terminator
    AriaFunc* funcs;
} AriaMod;

#define ARIA_FUNC_TERMINATOR \
    (AriaFunc) { .name = NULL, .f = NULL }

void ariaInit(AriaVM*);
void ariaStoreArgs(AriaVM*, int, char**);
void ariaPopulateMod(AriaMod*, AriaFunc[]);
int ariaDoString(AriaVM*, const char*);
int ariaDoFile(AriaVM*, const char*);
void ariaPushTop(AriaStack*, AriaObj*);
AriaObj ariaGetTop(AriaStack*);
void ariaCleanup(AriaVM*);

double ariaUnwrapNumObject(const AriaObj*);

#endif  // ARIA_H
