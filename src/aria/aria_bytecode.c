#include "aria_bytecode.h"

#include <assert.h>

#include "aria_lexer.h"
#include "aria_stack.h"
#include "nob.h"

void compileExpr(Aria_Chunk* chunk, ASTNode* node) {
    if (node->type == AST_NUM_LIT) {
        Aria_Bytecode bc = {.op = OP_STORE, .operand_1 = node->num_literal};
        nob_da_append(chunk, bc);
        return;

    } else if (node->type == AST_IDENT) {
        const size_t* index = ht_find(&chunk->symtab, node->identifier);
        Aria_Bytecode bc = {.op = OP_LOAD, .operand_1 = *index};
        nob_da_append(chunk, bc);
        return;

    } else if (node->type == AST_CALL) {
        nob_da_append(&chunk->mod->heap, node->funcCall.name);
        Aria_Bytecode bc = {.op = OP_CALL, .operand_1 = chunk->mod->heap.count - 1};
        nob_da_append(chunk, bc);
        return;

    } else if (node->type == AST_EXPR) {
        compileExpr(chunk, node->expr.lhs);
        compileExpr(chunk, node->expr.rhs);

        switch (node->expr.op) {
            case TOK_PLUS:
                nob_da_append(chunk, (Aria_Bytecode){.op = OP_ADD});
                break;
            case TOK_MINUS:
                nob_da_append(chunk, (Aria_Bytecode){.op = OP_SUB});
                break;
            case TOK_STAR:
                nob_da_append(chunk, (Aria_Bytecode){.op = OP_MUL});
                break;
            case TOK_SLASH:
                nob_da_append(chunk, (Aria_Bytecode){.op = OP_DIV});
                break;
            case TOK_LESS:
                nob_da_append(chunk, (Aria_Bytecode){.op = OP_LT});
                break;
            case TOK_GREATER:
                nob_da_append(chunk, (Aria_Bytecode){.op = OP_GT});
                break;
            case TOK_LESS_EQUAL:
                nob_da_append(chunk, (Aria_Bytecode){.op = OP_LE});
                break;
            case TOK_GREATER_EQUAL:
                nob_da_append(chunk, (Aria_Bytecode){.op = OP_GE});
                break;
            case TOK_EQUAL_EQUAL:
                nob_da_append(chunk, (Aria_Bytecode){.op = OP_EQ});
                break;
            case TOK_BANG_EQUAL:
                nob_da_append(chunk, (Aria_Bytecode){.op = OP_NE});
                break;
            default:
                NOB_UNREACHABLE("Unhandled operation in expr");
        }

        return;
    }

    NOB_UNREACHABLE("Expr called incorrectly");
}

void compileIf(Aria_Chunk* chunk, ASTNode* node, bool inner) {
    // handle condition
    compileExpr(chunk, node->If.cond);
    const ASTNode* ifBlock = node->If.block;
    Aria_Bytecode jump = {.op = OP_JUMP_COND, .operand_1 = ifBlock->block.count + 1};
    nob_da_append(chunk, jump);

    // IF block
    for (size_t i = 0; i < ifBlock->block.count; i++) {
        ASTNode* item = &ifBlock->block.items[i];
        compileStmt(chunk, item);
    }

    // Else block
    Aria_Chunk tempElseChunk = {0};
    ASTNode* elseBlock = node->If.elseBlock;
    if (elseBlock->type == AST_IF) {
        compileIf(chunk, elseBlock, true);
    } else {
        for (size_t i = 0; i < elseBlock->block.count; i++) {
            ASTNode* item = &elseBlock->block.items[i];
            compileStmt(&tempElseChunk, item);
        }
    }

    if (!inner) {
        Aria_Bytecode jump2 = {.op = OP_JUMP, .operand_1 = tempElseChunk.count};
        nob_da_append(chunk, jump2);
    }

    nob_da_append_many(chunk, tempElseChunk.items, tempElseChunk.count);
}

void compileVar(Aria_Chunk* chunk, ASTNode* node) {
    switch (node->var.ret_type) {
        case TOK_BOOL:
            break;
        case TOK_CHAR:
            break;
        case TOK_NUM:
            const int stack_sz = stackSize(chunk->mod->stack);
            *ht_put(&chunk->symtab, node->var.name) = stack_sz;
            printf("ht_put: %s\n", node->var.name);
            Aria_Bytecode bc = {.op = OP_STORE,
                                .operand_1 = node->var.value->num_literal,
                                .operand_2 = stack_sz,
                                .operand_count = 2};
            nob_da_append(chunk, bc);
            break;
        case TOK_STR:
            break;
        default:
            // class/type
    }
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
            compileIf(chunk, node, false);
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
            compileExpr(chunk, node->ret.expr);
            nob_da_append(chunk, (Aria_Bytecode){.op = OP_RETURN});
        } break;
        case AST_STR_LIT:
            break;
        case AST_TYPE:
            break;
        case AST_VAR:
            compileVar(chunk, node);
            break;
        default:
            break;
    }
}

Aria_Chunk compileFunc(ASTNode* node, Aria_Module* mod) {
    Aria_Chunk chunk = {0};
    chunk.name = node->func.name;
    chunk.stackStart = stackSave(mod->stack);
    chunk.symtab = (Symbol_table_t){.hasheq = ht_cstr_hasheq};
    chunk.mod = mod;

    // add every param to the symtab
    for (size_t i = 0; i < node->func.args.count; i++) {
        const ASTNode* arg = &node->func.args.items[i];
        *ht_put(&chunk.symtab, arg->arg.name) = stackSize(chunk.mod->stack);
    }

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
    mod.chunks = (Chunk_map_t){.hasheq = ht_cstr_hasheq};
    mod.stack = createStack(NOB_DA_INIT_CAP);

    nob_da_foreach(ASTNode, node, &ast.block) {
        switch (node->type) {
            case AST_FUNC:
                Aria_Chunk c = compileFunc(node, &mod);
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
    printf("=== BYTECODE ===\n");
    printf("Module: %s\n", mod->name);

    ht_foreach(chunk, &mod->chunks) {
        printf("  Chunk: %s\n", ht_key(&mod->chunks, chunk));

        nob_da_foreach(Aria_Bytecode, i, chunk) {
            printf("    %s (%ld, %ld, %ld)\n", opcodeName(i->op), i->operand_1, i->operand_2,
                   i->operand_3);
        }
    }

    printf("\n");
}

char* opcodeName(Opcode op) {
    switch (op) {
        case OP_RETURN:
            return "OP_RETURN";
        case OP_STORE:
            return "OP_STORE";
        case OP_ADD:
            return "OP_ADD";
        case OP_SUB:
            return "OP_SUB";
        case OP_MUL:
            return "OP_MUL";
        case OP_DIV:
            return "OP_DIV";
        case OP_CALL:
            return "OP_CALL";
        case OP_JUMP_COND:
            return "OP_JUMP_COND";
        case OP_JUMP:
            return "OP_JUMP";
        case OP_LOAD:
            return "OP_LOAD";
        case OP_LT:
            return "OP_LT";
        case OP_GT:
            return "OP_GT";
        case OP_LE:
            return "OP_LE";
        case OP_GE:
            return "OP_GE";
        case OP_EQ:
            return "OP_EQ";
        case OP_NE:
            return "OP_NE";
        case OP_STACK_REWIND:
            return "OP_STACK_REWIND";
        case OP_STACK_SAVE:
            return "OP_STACK_SAVE";
    }

    return "";
}
