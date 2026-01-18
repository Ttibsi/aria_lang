#include "aria.h"

#define NOB_IMPLEMENTATION
#include "../include/nob.h"

void ariaInit(AriaVM* vm) {
    AriaStack stack = {0};
    vm->global_stack = stack;
}

void ariaStoreArgs(AriaVM* vm, int argc, char** argv) {
    vm->argc = argc;
    vm->argv = argv;
}

void ariaPopulateMod(AriaMod* mod, AriaFunc* funcs) { mod->funcs = funcs; }

int ariaDoString(AriaVM* vm, const char* str) {
    ariaLexerInit(&vm->lexer, str);
    ariaTokenize(&vm->lexer);

    if (vm->debug_mode) { printTokens(&vm->lexer); }

    NOB_TODO("ariaDoString not implemented");
}

int ariaDoFile(AriaVM* vm, const char* filename) {
    Nob_String_Builder file_buf = {0};
    if (!nob_read_entire_file(filename, &file_buf)) { return 1; }
    nob_sb_append_null(&file_buf);

    return ariaDoString(vm, file_buf.items);
}

void ariaPushTop(AriaStack* stk, AriaObj* obj) { nob_da_append(stk, *obj); }

AriaObj ariaGetTop(AriaStack* stk) {
    AriaObj obj = nob_da_last(stk);
    stk->count--;
    return obj;
}

double ariaUnwrapNumObject(AriaObj* obj) {
    if (obj->type == OBJ_TYPE_NUM) { return obj->as.i; };

    return 0.0;
}

void ariaCleanup(AriaVM* vm) {
    // TODO: DO we free the lexer here? Or before this
    nob_da_free(vm->global_stack);
    vm = NULL;
}
