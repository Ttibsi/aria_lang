#include "aria.h"

#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>

char* file_to_string(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) return NULL;
    
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char* str = malloc(len + 1);
    if (!str) { fclose(f); return NULL; }
    
    fread(str, 1, len, f);
    str[len] = '\0';
    fclose(f);
    
    return str;
}

int main(int argc, char* argv[]) {
    Aria_VM ariaVM = aria_vm_init(); 

    if (argc > 1) {
        const char* mod_name = basename(argv[1]);
        const char* text = file_to_string(argv[1]);
        aria_interpret(&ariaVM, mod_name, text);
    } else {
        // TODO: Proper repl
        aria_interpret(&ariaVM, "main", "var foo = 35 + (2 * 17);");
    }

    aria_vm_destroy(&ariaVM); 
    return 0; 
} 
