#include <stdio.h>

#define NOB_STRIP_PREFIX
#define NOB_IMPLEMENTATION
#include "include/nob.h"

void usage() { printf("No valid commands currently"); }

int main(int argc, char** argv) {
    GO_REBUILD_URSELF(argc, argv);

    if (argc > 1) {
        usage();
        return 1;
    }

    mkdir_if_not_exists("build/");

    Cmd cmd = {0};
    cmd_append(&cmd, "gcc");
    nob_cc_flags(&cmd);
    cmd_append(&cmd, "-std=c99");
    cmd_append(&cmd, "-g");
    cmd_append(&cmd, "src/aria.c");
    cmd_append(&cmd, "src/main.c");
    nob_cc_output(&cmd, "build/aria");

    if (!cmd_run(&cmd)) { return 1; }
    return 0;
}
