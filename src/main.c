#include <stdio.h>
#include "aria/aria.h"

char* file_to_string(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "File does not exist\n");
        return NULL;
    }

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
    bool debug_mode = false;

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-D") == 0) {
            debug_mode = true;
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
        char* text = file_to_string(filename);
        if (text == NULL) { return 1; }
        const int ret = ariaInterpret(text, debug_mode);
        free(text);
        return ret;
    } else {
        // TODO: Proper repl
        usage();
    }

    return 0;
}
