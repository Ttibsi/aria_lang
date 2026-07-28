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

static inline int test_compileIf(void) {
    ASTNode lhs = (ASTNode){.type = AST_NUM_LIT, .num_literal = 3};
    ASTNode rhs = (ASTNode){.type = AST_NUM_LIT, .num_literal = 5};
    ASTNode cond = (ASTNode){.type = AST_EXPR, .expr = {.op = TOK_LESS, .lhs = &lhs, .rhs = &rhs}};

    ASTNode ifRetExpr = (ASTNode){.type = AST_NUM_LIT, .num_literal = 69};
    ASTNode ifRet = (ASTNode){.type = AST_RETURN, .ret = {.expr = &ifRetExpr}};
    ASTNode ifBlock = ariaCreateNode(AST_BLOCK);
    nob_da_append(&ifBlock.block, ifRet);

    ASTNode elseRetExpr = (ASTNode){.type = AST_NUM_LIT, .num_literal = 42};
    ASTNode elseRet = (ASTNode){.type = AST_RETURN, .ret = {.expr = &elseRetExpr}};
    ASTNode elseBlock = ariaCreateNode(AST_BLOCK);
    nob_da_append(&elseBlock.block, elseRet);

    ASTNode n = (ASTNode){
        .type = AST_IF,
        .If = {
            .cond = &cond,
            .block = &ifBlock,
            .elseBlock = &elseBlock,
        },
    };
    Aria_Chunk c = {0};

    compileIf(&c, &n, false);

    onetest_assert(c.count == 9);
    onetest_assert(c.items[0].op == OP_STORE);
    onetest_assert(c.items[0].operand_1 == 5);
    onetest_assert(c.items[1].op == OP_STORE);
    onetest_assert(c.items[1].operand_1 == 3);
    onetest_assert(c.items[2].op == OP_LT);
    onetest_assert(c.items[3].op == OP_JUMP_COND);
    onetest_assert(c.items[3].operand_1 == 2);
    onetest_assert(c.items[4].op == OP_STORE);
    onetest_assert(c.items[4].operand_1 == 69);
    onetest_assert(c.items[5].op == OP_RETURN);
    onetest_assert(c.items[6].op == OP_JUMP);
    onetest_assert(c.items[6].operand_1 == 2);
    onetest_assert(c.items[7].op == OP_STORE);
    onetest_assert(c.items[7].operand_1 == 42);
    onetest_assert(c.items[8].op == OP_RETURN);

    return 0;
}

static inline int test_compileVar(void) {
    Aria_Chunk c = {0};

    ASTNode n = (ASTNode){
        .type = AST_VAR,
        .var.name = "foo",
    };
    n.var.value = malloc(sizeof(ASTNode));
    n.var.value->num_literal = 5;
    compileVar(&c, &n);
    Aria_Bytecode bc = *c.items;

    onetest_assert(bc.op == OP_STORE);
    onetest_assert(bc.operand_1 == 5);
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
