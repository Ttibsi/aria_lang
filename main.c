#include "src/aria.h"

#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>


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

void usage() {
    fprintf(stderr, "Usage: ./aria <filename>\n");
}

int main(int argc, char* argv[]) {
    const char* filename = NULL;

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-D") == 0) {
            aria_debug_mode = 1;
        } else if (argv[i][0] != '-') {
            // This is the filename (first non-flag argument)
            filename = argv[i];
            break;
        } else {
            // Unknown flag
            fprintf(stderr, "Unknown flag: %s\n", argv[i]);
            usage();
            return 1;
        }
    }

    if (filename != NULL) {
        const char* text = file_to_string(filename);
        ariaInterpret(text);
    } else {
        // TODO: Proper repl
        usage();
    }

    return 0;
}
