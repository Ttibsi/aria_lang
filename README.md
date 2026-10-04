# Aria
A simple scripting language with BASIC-like syntax that is designed to be
embedded into other applications, written in standard C23 with no compiler
extensions.

Note that this project is in early development. There are examples of syntax
in the `examples` directory, although not all of them successfully run through
the interpreter. See the `Dev Progress` section of this README below.

# Requirements and building
This project uses the [nob.h](https://github.com/tsoding/nob.h) build system.

```console
$ gcc nob.c -o nob
$ ./nob

# To compile tests
$ ./nob test
$ ./build/test_exe
```

The two above shell commands will compile the build script, and then the
interpreter. The second set of commands above will compile and execute the
test suite for this codebase.

### Dev Progress
See `examples` directory for each example of different code snippets used for
development. The progress is as follows:

[ ] 01_ret.ari
    - Status: Compiles and executes
[ ] 02_func_call.ari
    - Status: Compiles to bytecode
[ ] 03_variables.ari
    - Status: Compiles to bytecode
[ ] 04_if.ari
    - Status: Compiles to bytecode
[ ] 05_for.ari
    - Status: Frontend only
[ ] 06_types.ari
    - Status: Frontend only
[ ] 07_imports.ari
    - Status: Frontend only

Other examples and further documentation to come, as well as a more
all-encompassing test suite
