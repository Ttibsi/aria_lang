#include "aria_execute.h"
#include "onetest.h"

static inline int test_executeInstruction(void) {
    Stack* local = createStack(8);

    Aria_Bytecode* inst = malloc(sizeof(Aria_Bytecode));
    inst->op = OP_STORE_CONST;
    inst->operand = 42;
    inst->next = NULL;
    inst->prev = NULL;

    executeInstruction(NULL, local, inst);

    onetest_assert(stackPop(local) == 42);

    freeStack(local);
    free(inst);
    return 0;
}

static inline int test_executeFunction(void) {
    Stack* global = createStack(8);

    Aria_Lexer L = lexerInit("func main() { return 69; }");
    ariaTokenize(&L);
    ASTNode ast = ariaParse(&L);
    Aria_Module* mod = ariaCompile(&ast);

    const int ret = executeFunction(global, (Aria_Chunk*)bufferGet(mod->buf, 0));
    onetest_assert(ret == 69);

    freeStack(global);
    return 0;
}

static inline int test_ariaExecute(void) {
    Aria_Lexer L = lexerInit("func main() { return 69; }");
    ariaTokenize(&L);
    ASTNode ast = ariaParse(&L);
    Aria_Module* mod = ariaCompile(&ast);

    Stack* stack = ariaExecute(mod);
    onetest_assert(stackPop(stack) == 69);

    freeStack(stack);
    return 0;
}
