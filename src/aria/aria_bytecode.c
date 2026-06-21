#include "aria_bytecode.h"

#include <assert.h>

#include "nob.h"

Aria_Bytecode compileExpr(ASTNode* node) {
    if (node->type == AST_NUM_LIT) {
        return (Aria_Bytecode){.op = OP_STORE, .operand_1 = node->num_literal};
    }

    NOB_UNREACHABLE("Expr called incorrectly");
}

void compileStmt(Aria_Chunk* chunk, ASTNode* node) {
    switch (node->type) {
        case AST_ARG:
            break;
        case AST_ASSIGN:
            break;
        case AST_BLOCK:
            break;
        case AST_CALL:
            break;
        case AST_CHAR_LIT:
            break;
        case AST_ERR:
            break;
        case AST_EXPR:
            break;
        case AST_FOR:
            break;
        case AST_FOREACH:
            break;
        case AST_FUNC:
            break;
        case AST_IDENT:
            break;
        case AST_IF:
            break;
        case AST_IMPORT:
            break;
        case AST_METHOD_CALL:
            break;
        case AST_MODULE:
            break;
        case AST_NUM_LIT:
            break;
        case AST_RETURN: {
            nob_da_append(chunk, compileExpr(node->ret.expr));
            nob_da_append(chunk, (Aria_Bytecode){.op = OP_RETURN});
        } break;
        case AST_STR_LIT:
            break;
        case AST_TYPE:
            break;
        case AST_VAR:
            break;
        default:
            break;
    }
}

Aria_Chunk compileFunc(ASTNode* node) {
    Aria_Chunk chunk = {0};
    chunk.name = node->func.name;

    for (size_t i = 0; i < node->func.body->block.count; i++) {
        ASTNode* item = &node->func.body->block.items[i];
        compileStmt(&chunk, item);
    }

    return chunk;
}

Aria_Module ariaEmitBytecode(ASTNode ast) {
    assert(ast.type == AST_MODULE);

    Aria_Module mod = {0};
    mod.name = ast.block.name;

    nob_da_foreach(ASTNode, node, &ast.block) {
        switch (node->type) {
            case AST_FUNC:
                Aria_Chunk c = compileFunc(node);
                *ht_put(&mod.chunks, c.name) = c;
                break;
            case AST_IMPORT:
                break;
            case AST_TYPE:
                break;

            case AST_ERR:
                [[fallthrough]];
            default:
        }
    }

    return mod;
}

void printBytecode(Aria_Module* mod) {
    printf("=== BYTECODE ===");
    printf("Module: %s\n", mod->name);

    ht_foreach(chunk, &mod->chunks) {
        printf("  Chunk: %s\n", ht_key(&mod->chunks, chunk));

        nob_da_foreach(Aria_Bytecode, i, chunk) {
            printf("    %s (%ld, %ld, %ld)\n", opcodeName(i->op), i->operand_1, i->operand_2,
                   i->operand_3);
        }
    }
}

char* opcodeName(Opcode op) {
    switch (op) {
        case OP_RETURN:
            return "OP_RETURN";
        case OP_STORE:
            return "OP_STORE";
    }

    return "";
}
