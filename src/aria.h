#ifndef ARIA_H
#define ARIA_H

typedef enum {
    STATUS_OK,
    STATUS_SOME_ERR,
} Status;

// Forward declarations for VM
struct String;
struct Strings;
struct Table;

typedef struct {
    // hash table of AriaFunctions with their names as the key used for function calls
    struct Table* functions;

    struct ValueArray* stack;
    struct ValueArray* callstack;  // Every object should be a pointer to an AriaFunction

    DA(String*, Strings);  // Easier to keep track of and free all SVs at once
} AriaVM;

void aria_vm_cleanup(AriaVM* vm);
Status aria_load_file(AriaVM* vm, const char* filepath);
Status aria_call_func(AriaVM* vm, const char* func);

#endif  // ARIA_H
