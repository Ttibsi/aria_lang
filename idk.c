#include "idk.h"

// these are the public-only functions 

void aria_vm_init(Aria_VM* aria_vm, char* in_str, int length) {
    aria_vm->arena = arena_init(16 *1024); 
}

void aria_vm_destroy(Aria_VM* aria_vm) {
}

void aria_dostring(Aria_VM* aria_vm) {
} 
 
int main() {
    Aria_VM ariaVM = {0};
    aria_vm_init(&ariaVM, in_str, sizeof(in_str)); 

    aria_dostring(&ariaVM);

    /* ... */

    aria_vm_destroy(&ariaVM); 
    return 0; 
} 
