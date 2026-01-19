#include "aria_parser.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>

#include "nob.h"

#define parsingError(str, ...)                                                               \
    do {                                                                                     \
        fprintf(stderr, "%s:%d - " str "\n", __func__, __LINE__ __VA_OPT__(, ) __VA_ARGS__); \
        assert(0);                                                                           \
    } while (0)

binding_t prefixBindingPower(const TokenType* tkn) {
    switch (*tkn) {
        case TOK_BANG:
        case TOK_MINUS:
            return 8;
            break;
        default:
            return 0;
    }

    return 0;
}

binding_t infixBindingPower(const TokenType* tkn) {
    switch (*tkn) {
        case TOK_DOT:
        case TOK_LEFT_PAREN:
        case TOK_RIGHT_PAREN:
            return 9;
        case TOK_STAR:
        case TOK_SLASH:
            return 7;
        case TOK_PLUS:
        case TOK_MINUS:
            return 6;
        case TOK_GREATER:
        case TOK_GREATER_EQUAL:
        case TOK_LESS:
        case TOK_LESS_EQUAL:
            return 5;
        case TOK_EQUAL_EQUAL:
        case TOK_BANG_EQUAL:
            return 4;
        case TOK_AND:
            return 3;
        case TOK_OR:
            return 2;
        case TOK_EQUAL:
            return 1;
        default:
            return 0;
    };

    return 0;
}

ASTNode parseBlock(AriaLexer* L) {
    ASTNode node = ariaCreateNode(AST_BLOCK);
    if (!match(L, TOK_LEFT_BRACE)) { parsingError("Entered a block inappropriately"); }

    while (!check(L, TOK_RIGHT_BRACE)) {
        ASTNode stmt = parseStatement(L);
        nob_da_append(&node.block, stmt);
    }

    return node;
}

ASTNode parseFunc(AriaLexer* L) {
    if (!match(L, TOK_FUNC)) { parsingError("parseFunc called with incorrect token\n"); }
    advance(L);

    ASTNode funcNode = ariaCreateNode(AST_FUNC);

    // func name
    const AriaToken* tok = &L->items[L->index];
    Nob_String_Builder name = {0};
    nob_sb_append_buf(&name, &L->source[tok->start], tok->len);
    nob_sb_append_null(&name);
    funcNode.func.name = name.items;
    advance(L);

    // arguments
    // All default to 0 values, akin to `AriaToken args[i] = {0}`
    if (!match(L, TOK_LEFT_PAREN)) { parsingError("Function name not followed by open bracket\n"); }
    int args_idx = 0;
    while (!check(L, TOK_RIGHT_PAREN)) {
        if (!check(L, TOK_IDENTIFIER)) { parsingError("function args contain non-identifiers\n"); }
        if (args_idx >= param_count) { parsingError("Function has too many arguments\n"); }

        ASTNode* arg = malloc(sizeof(ASTNode));

        Nob_String_Builder arg_name = {0};
        nob_sb_append_buf(&arg_name, &L->source[tok->start], tok->len);
        nob_sb_append_null(&arg_name);
        arg->arg.name = arg_name.items;
        advance(L);

        arg->arg.type = L->items[L->index].type;
        funcNode.func.args[args_idx] = arg;

        args_idx++;
        advance(L);
        if (check(L, TOK_COMMA)) { advance(L); }  // Skip commas
    }
    advance(L);

    // return type
    funcNode.func.ret_type = L->items[L->index].type;
    advance(L);

    // body
    ASTNode* body = malloc(sizeof(ASTNode));
    *body = parseBlock(L);
    funcNode.func.body = body;

    return funcNode;
}

ASTNode parseExpression(AriaLexer* L, const binding_t min_bp) {}
ASTNode parseFor(AriaLexer* L) {}
ASTNode parseIdentifier(AriaLexer* L) {}
ASTNode parseIf(AriaLexer* L) {}

ASTNode parseImport(AriaLexer* L) {
    ASTNode node = ariaCreateNode(AST_IMPORT);
    advance(L);
    if (!(match(L, TOK_IDENTIFIER) || match(L, TOK_STRING_LIT))) {
        parsingError("Import statement invalid");
    }

    if (match(L, TOK_STRING_LIT)) { node.import.local_file = true; }

    const AriaToken* tok = &L->items[L->index];
    Nob_String_Builder name = {0};
    nob_sb_append_buf(&name, &L->source[tok->start], tok->len);
    nob_sb_append_null(&name);
    node.import.name = name.items;

    return node;
}

ASTNode parseReturn(AriaLexer* L) {
    ASTNode node = ariaCreateNode(AST_RETURN);
    advance(L);

    ASTNode* expr = malloc(sizeof(ASTNode));
    *expr = parseExpression(L, 0);

    if (expr->type == AST_EXPR) {
        node.ret.expr = expr;
    } else {
        node.ret.expr = malloc(sizeof(ASTNode));
        node.ret.expr->type = AST_EXPR;
        node.ret.expr->expr.lhs = expr;
        node.ret.expr->expr.rhs = NULL;
        node.ret.expr->expr.op = expr->type;
        advance(L);
    }

    match(L, TOK_SEMICOLON);  // consume semicolon if needed
    return node;
}

ASTNode parseStatement(AriaLexer* L) {
    AriaToken* tkn = &L->items[L->index];
    switch (tkn->type) {
        case TOK_FOR:
            return parseFor(L);
            break;
        case TOK_IF:
            return parseIf(L);
            break;
        case TOK_RET:
            return parseReturn(L);
            break;
        case TOK_VAR:
            return parseVar(L);
            break;
        case TOK_IDENTIFIER:
            return parseIdentifier(L);
            break;
        default:
            parsingError("Incorrect token found in parseStatement (default)\n");
            break;
    };

    parsingError("Incorrect token found in parseStatement\n");
    return ERR_NODE;
}

ASTNode parseType(AriaLexer* L) {}
ASTNode parseVar(AriaLexer* L) {}

ASTNode ariaCreateNode(const NodeType type) {
    switch (type) {
        case AST_BLOCK:
            [[fallthrough]];
        case AST_MODULE:
            return (ASTNode){.type = type};

        case AST_RETURN:
            return (ASTNode){.type = type};

        case AST_IMPORT:
            return (ASTNode){.type = type, .import = {.local_file = false, .name = NULL}};

        case AST_ARG:
            return (ASTNode){.type = type,
                             .arg = {
                                 .name = NULL,
                             }};

        case AST_FUNC:
            return (ASTNode){.type = type, .func = {.name = NULL, .body = NULL}};

        case NODE_COUNT:
            [[fallthrough]];
        default:
            NOB_UNREACHABLE("Unhandled type in ariaCreateNode");
    }
}

ASTNode ariaParse(AriaLexer* L) {
    ASTNode module = ariaCreateNode(AST_MODULE);

    while (!check(L, TOK_EOF)) {
        ASTNode inner = (ASTNode){.type = AST_ERR};

        switch (getCurrTokenType(L)) {
            case TOK_FUNC:
                inner = parseFunc(L);
                advance(L);  // TOK_RIGHT_BRACE
                break;

            case TOK_IMPORT:
                inner = parseImport(L);
                break;

            case TOK_TYPE:
                inner = parseType(L);
                break;

            case TOK_ERROR:
                [[fallthrough]];
            default:
                NOB_UNREACHABLE("Invalid node at top level in ariaParse");
        };

        nob_da_append(&module.block, inner);
    }

    return module;
}

void printAst(const ASTNode* root) { (void)root; }
