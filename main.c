#include "idk.h"

int main() {
    Aria_VM ariaVM = {0};
    aria_vm_init(&ariaVM, in_str, sizeof(in_str)); 

    aria_interpret(&ariaVM, "main", "var foo = 35 + (2 * 17)");

    /* ... */

    aria_vm_destroy(&ariaVM); 
    return 0; 
} 
