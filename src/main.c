#include <assert.h>

#include "aria/aria.h"

int someFunc(AriaVM* vm) {}
void writeFn(AriaVM* vm) {}

int main(int argc, char* argv[]) {
    AriaVM vm = {0};
    ariaInit(&vm);
    ariaStoreArgs(&vm, argc, argv);
    vm.debug_mode = true;

    // setup user-defined functions on the VM
    // such as writing and standard memory management (malloc/free)
    vm.writeFn = writeFn;

    // Create a module that contains multiple functions
    AriaMod module = {0};
    module.name = "module_name";
    AriaFunc funcs[] = {
        (AriaFunc){.name = "someFunc", .f = someFunc},
        ARIA_FUNC_TERMINATOR,
    };
    ariaPopulateMod(&module, funcs);

    // Execute some code either as a string or from a file
    // const char* src
    //     = "import module_name\n\n"
    //       "print(module_name.someFunc())\n";
    // [[maybe_unused]] int retcode_str = ariaDoString(&vm, src);
    [[maybe_unused]] int retcode_file = ariaDoFile(&vm, "/home/auri/workspace/t.ari");

    // C <> lang value interop
    AriaObj obj = {.as = {69}};
    ariaPushTop(&vm.global_stack, &obj);
    const AriaObj value_obj = ariaGetTop(&vm.global_stack);
    const double value = ariaUnwrapNumObject(&value_obj);
    assert(value == 69);

    // Clean up any hanging memory
    ariaCleanup(&vm);
}
