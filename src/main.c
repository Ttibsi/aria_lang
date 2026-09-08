#include <stdio.h>

#include "aria.h"

int main() {
    AriaVM vm = {0};
    // TODO: initVM if needed
    // TODO: vm.debug_mode if needed

    const Status ret = aria_load_file(&vm, "t.ari");
    if (ret) {
        // Some error happened
    }

    const Status result = aria_call_func(&vm, "main");

    aria_vm_cleanup(&vm);

    printf("Hello world\n");
    return 0;
}
