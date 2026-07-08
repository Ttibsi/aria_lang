#include <string.h>

#include "aria/aria_bytecode.h"
#include "aria/aria_parser.h"
#include "ht.h"
#include "nob.h"
#include "onetest.h"

static inline int test_compileExpr(void) {
    Aria_Chunk c = {0};
    ASTNode n = (ASTNode){.type = AST_NUM_LIT, .num_literal = 69};
    compileExpr(&c, &n);
    Aria_Bytecode bc = *c.items;

    onetest_assert(bc.op == OP_STORE);
    onetest_assert(bc.operand_1 == 69);

    return 0;
}

static inline int test_compileStmt(void) {
    ASTNode inner = (ASTNode){.type = AST_NUM_LIT, .num_literal = 69};
    ASTNode n = (ASTNode){.type = AST_RETURN, .ret = {.expr = &inner}};
    Aria_Chunk c = {0};

    compileStmt(&c, &n);

    onetest_assert(c.count == 2);
    onetest_assert(c.items[0].op == OP_STORE);
    onetest_assert(c.items[1].op == OP_RETURN);
    return 0;
}

static inline int test_compileFunc(void) {
    ASTNode inner = (ASTNode){.type = AST_NUM_LIT, .num_literal = 69};
    ASTNode stmt = (ASTNode){.type = AST_RETURN, .ret = {.expr = &inner}};
    ASTNode body = ariaCreateNode(AST_BLOCK);
    nob_da_append(&body.block, stmt);
    ASTNode func = (ASTNode){.type = AST_FUNC, .func = {.name = "f", .body = &body}};

    const Aria_Chunk chunk = compileFunc(&func);

    onetest_assert(chunk.count == 2);
    onetest_assert(chunk.items[0].op == OP_STORE);
    onetest_assert(chunk.items[1].op == OP_RETURN);
    return 0;
}

static inline int test_ariaEmitBytecode(void) {
    ASTNode inner = (ASTNode){.type = AST_NUM_LIT, .num_literal = 69};
    ASTNode stmt = (ASTNode){.type = AST_RETURN, .ret = {.expr = &inner}};
    ASTNode body = ariaCreateNode(AST_BLOCK);
    nob_da_append(&body.block, stmt);
    ASTNode func = (ASTNode){.type = AST_FUNC, .func = {.name = "f", .body = &body}};

    ASTNode module_ast = ariaCreateNode(AST_MODULE);
    module_ast.block.name = "main";
    nob_da_append(&module_ast.block, func);

    const Aria_Module mod = ariaEmitBytecode(module_ast);

    onetest_assert(strcmp(mod.name, "main") == 0);

    const Aria_Chunk* f = ht_find(&mod.chunks, "f");
    onetest_assert(f != NULL);

    return 0;
}

static inline int test_opcodeName(void) {
    onetest_assert(strcmp(opcodeName(OP_RETURN), "OP_RETURN") == 0);
    onetest_assert(strcmp(opcodeName(OP_STORE), "OP_STORE") == 0);
    return 0;
}
