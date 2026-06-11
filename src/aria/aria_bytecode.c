#include "aria_bytecode.h"

#include <assert.h>

#include "../include/nob.h"

Aria_Module ariaEmitBytecode(ASTNode ast) {
    assert(ast.type == AST_MODULE);

    Aria_Module mod = {0};
    mod.name = ast.block.name;

    nob_da_foreach(ASTNode, node, &ast.block) {
        switch (node->type) {
            case TOK_FUNC:
                break;
            case TOK_IMPORT:
                break;
            case TOK_TYPE:
                break;

            case TOK_ERROR:
                [[fallthrough]];
            default:
        }
    }

    return mod;
}
