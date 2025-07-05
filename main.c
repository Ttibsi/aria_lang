#include "aria.h"

int main() {
    Aria_VM ariaVM = aria_vm_init(); 
    aria_interpret(&ariaVM, "main", "var foo = 35 + (2 * 17);");

    /* ... */

    aria_vm_destroy(&ariaVM); 
    return 0; 
} 
