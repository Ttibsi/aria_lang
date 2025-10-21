#include "aria_parser.h"

#include <assert.h>
#include <stdio.h>
#include <stddef.h>

#define parsingError(str, ...) \
    do { \
        fprintf(stderr, "%s:%d - "str"\n", __func__, __LINE__ __VA_OPT__(,) __VA_ARGS__); \
        assert(0); \
    } while(0)

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
        case TOK_DOT: case TOK_LEFT_PAREN: case TOK_RIGHT_PAREN:
            return 9;
        case TOK_STAR: case TOK_SLASH:
            return 7;
        case TOK_PLUS: case TOK_MINUS:
            return 6;
        case TOK_GREATER: case TOK_GREATER_EQUAL: case TOK_LESS: case TOK_LESS_EQUAL:
            return 5;
        case TOK_EQUAL_EQUAL: case TOK_BANG_EQUAL:
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

ASTNode parseConst(Aria_Lexer* L) { assert(0 && "TODO"); }
ASTNode parseFor(Aria_Lexer* L) { assert(0 && "TODO"); }
ASTNode parseIf(Aria_Lexer* L) { assert(0 && "TODO"); }

ASTNode parseReturn(Aria_Lexer* L) {
    ASTNode node = createNode(AST_RETURN);
    advance(L);

    ASTNode* expr = malloc(sizeof(ASTNode));
    *expr = parseExpression(L, 0);
    node.ret.expr = expr;

    advance(L); // consume semicolon
    return node;
}

ASTNode parseSwitch(Aria_Lexer* L) { assert(0 && "TODO"); }
ASTNode parseVar(Aria_Lexer* L) { assert(0 && "TODO"); }

ASTNode parseFuncCall(Aria_Lexer* L) {
    // NOTE: When this is called, the index of the buffer is on the oparen
    ASTNode node = createNode(AST_FUNCCALL);

    // function name
    Aria_Token* func_call = (Aria_Token*)bufferGet(L->tokens, L->buf_index - 1);
    node.func_call.func_name = malloc(sizeof(char) * func_call->len + 1);
    memcpy(node.func_call.func_name, L->source + func_call->start, func_call->len);
    node.func_call.func_name[func_call->len] = '\0';

    // parameters
    if (!match(L, TOK_LEFT_PAREN)) { parsingError("Function name not followed by open bracket\n"); }
    int args_idx = 0;
    while (!check(L, TOK_RIGHT_PAREN)) {
        if (!check(L, TOK_IDENTIFIER)) { parsingError("function args contain non-identifiers\n"); }
        if (args_idx >= param_count) { parsingError("Function has too many arguments\n"); }

        node.func.args[args_idx] = *(Aria_Token*)bufferGet(L->tokens, L->buf_index);
        args_idx++;
        advance(L);
        if (check(L, TOK_COMMA)) { advance(L); } // Skip commas
    }

    //advance(L); // TOK_RIGHT_PAREN
    return node;
}

ASTNode parseIdentifier(Aria_Lexer* L) {
    // const Aria_Token* ident = (Aria_Token*)bufferGet(L->tokens, L->buf_index);
    advance(L);

    switch(getCurrTokenType(L)) {
        case TOK_LEFT_PAREN: return parseFuncCall(L); break;
        case TOK_EQUAL:
            // Probably modifying a variable
            break;
    };

    unreachable();
}

ASTNode parseExpression(Aria_Lexer* L, binding_t min_bp) {
    ASTNode lhs = {};

    switch (getCurrTokenType(L)) {
        case TOK_NUMBER:
            lhs = createNode(AST_VALUE);
            lhs.value = getTokenNumber(L, (Aria_Token*)bufferGet(L->tokens, L->buf_index));
            break;
        case TOK_LEFT_PAREN:
            advance(L);
            lhs = parseExpression(L, 0);
            break;
        case TOK_IDENTIFIER:
            // TODO: This could also be a variable
            advance(L);
            if (check(L, TOK_LEFT_PAREN)) {
                lhs = parseFuncCall(L);
            } else {
                // TODO: parseVariable;
            }
            break;
        default:
            parsingError("Unknown token found (parseExpression LHS): %d", getCurrTokenType(L));
            break;
    };


    const Aria_Token* next = bufferGet(L->tokens, L->buf_index + 1);
    if (next->type == TOK_EOF) {
        parsingError("EOF reached when parsing an expression");
    } else if (next->type == TOK_SEMICOLON || next->type == TOK_RIGHT_PAREN) {
        return lhs;
    }

    while (true) {
        advance(L);
        const TokenType tok_type = getCurrTokenType(L);
        if (tok_type == TOK_SEMICOLON || tok_type == TOK_RIGHT_PAREN) {
            break;
        }

        const binding_t bp = infixBindingPower(&tok_type);
        if (bp < min_bp) { break; }
        advance(L);

        ASTNode rhs = parseExpression(L, bp + 1);

        ASTNode* lhs_ptr = malloc(sizeof(ASTNode));
        ASTNode* rhs_ptr = malloc(sizeof(ASTNode));
        *lhs_ptr = lhs;
        *rhs_ptr = rhs;

        lhs = createNode(AST_EXPR);
        lhs.expr.op = tok_type;
        lhs.expr.lhs = lhs_ptr;
        lhs.expr.rhs = rhs_ptr;
    }

    return lhs;
}

ASTNode parseClass(Aria_Lexer* L) { assert(0 && "TODO"); }
ASTNode parseExport(Aria_Lexer* L) { assert(0 && "TODO"); }
ASTNode parseImport(Aria_Lexer* l) { assert(0 && "TODO"); }

ASTNode parseBlock(Aria_Lexer* L) {
    ASTNode node = createNode(AST_BLOCK);
    advance(L); // TOK_LEFT_BRACE

    // NOTE: I'm not 100% happy with this logic -- nested blocks won't be handled well
    while (!(
        check(L, TOK_FUNC) ||
        check(L, TOK_CLASS) ||
        check(L, TOK_EXPORT) ||
        check(L, TOK_IMPORT)
    )) {
        ASTNode expr = parseStatement(L);
        bufferInsert(node.block, (void*)&expr);

        if (check(L, TOK_RIGHT_BRACE)) { break; }
    }

    return node;
}

ASTNode parseFunc(Aria_Lexer* L) {
    if (!match(L, TOK_FUNC)) { parsingError("parseFunc called with incorrect token\n"); }
    ASTNode node = createNode(AST_FUNC);

    // function name
    Aria_Token* curr_token = bufferGet(L->tokens, L->buf_index);
    node.func.func_name = malloc(sizeof(char) * curr_token->len + 1);
    memcpy(node.func.func_name, L->source + curr_token->start, curr_token->len);
    node.func.func_name[curr_token->len] = '\0';
    advance(L);

    // arguments
    if (!match(L, TOK_LEFT_PAREN)) { parsingError("Function name not followed by open bracket\n"); }
    int args_idx = 0;
    while (!check(L, TOK_RIGHT_PAREN)) {
        if (!check(L, TOK_IDENTIFIER)) { parsingError("function args contain non-identifiers\n"); }
        if (args_idx >= 8) { parsingError("Function has too many arguments\n"); }

        node.func.args[args_idx] = *(Aria_Token*)bufferGet(L->tokens, L->buf_index);
        args_idx++;
        advance(L);
        if (check(L, TOK_COMMA)) { advance(L); } // Skip commas
    }

    advance(L); // TOK_RIGHT_PAREN

    // Fill the rest of the 8 arguments
    while (args_idx < 8) {
        node.func.args[args_idx] = (Aria_Token){false, TOK_ERROR, -1, -1};
        args_idx++;
    }

    // body
    ASTNode* body = malloc(sizeof(ASTNode));
    *body = parseBlock(L);
    node.func.body = body;

    return node;
}

ASTNode parseStatement(Aria_Lexer* L) {
    Aria_Token* tkn = (Aria_Token*)bufferGet(L->tokens, L->buf_index);
    switch (tkn->type) {
        case TOK_CONST: return parseConst(L); break;
        case TOK_FOR: return parseFor(L); break;
        case TOK_IF: return parseIf(L); break;
        case TOK_RETURN: return parseReturn(L); break;
        case TOK_SWITCH: return parseSwitch(L); break;
        case TOK_VAR: return parseVar(L); break;
        case TOK_IDENTIFIER: return parseIdentifier(L); break;
        default: parsingError("Incorrect token found in parseStatement (defauLt)\n"); break;
    };

    parsingError("Incorrect token found in parseStatement\n");
    return (ASTNode){.type = AST_ERR};
}

ASTNode ariaParse(Aria_Lexer* L) {
    ASTNode module = createNode(AST_MODULE);

    while (!check(L, TOK_EOF)) {
        ASTNode inner = (ASTNode){.type = AST_ERR};

        switch (getCurrTokenType(L)) {
            case TOK_CLASS:
                inner = parseClass(L);
                break;
            case TOK_EXPORT:
                inner = parseExport(L);
                break;
            case TOK_FUNC:
                inner = parseFunc(L);
                advance(L); // TOK_RIGHT_BRACE
                break;
            case TOK_IMPORT:
                inner = parseImport(L);
                break;
            case TOK_ERROR:
            default:
                break;
        };

        bufferInsert(module.block, &inner);
    }

    return module;
}

ASTNode createNode(ASTType type) {
    switch (type) {
        case AST_BLOCK:
            [[fallthrough]];
        case AST_MODULE:
            return (ASTNode){
                .type = type,
                .block = bufferCreate(sizeof(ASTNode), 32)
            };

        case AST_FUNC:
            return (ASTNode){
                .type = type,
                .func = {
                    .func_name = NULL,
                    .body = NULL
                }
            };

        case AST_VALUE:
            return (ASTNode){
                .type = type,
                .value = 0
            };

        case AST_FUNCCALL:
            return (ASTNode){
                .type = type,
                .func_call = { .func_name = NULL }
            };

        case AST_RETURN:
            return (ASTNode){
                .type = type
            };

        case AST_EXPR:
            return (ASTNode){
                .type = type,
                .expr = {TOK_ERROR, NULL, NULL}
            };

        case AST_ERR:
            return (ASTNode){ .type = type };

        };

    return (ASTNode){.type = type};
}

void printASTimpl(ASTNode ast, int indent, Aria_Lexer* L) {
    switch (ast.type) {
        case AST_BLOCK: [[fallthrough]]; case AST_MODULE:
            Aria_Buffer* buf = ast.block;
            for (uint32_t i = 0; i < buf->size; i++) {
                printASTimpl(*(ASTNode*)bufferGet(buf, i), indent, L);
            }
            break;

        case AST_FUNC: case AST_FUNCCALL:
            printf("%*s@Function Node: { %s }\n", indent, "", ast.func.func_name);

            // Arguments
            for(int i = 0; i < 8; i++) {
                if (ast.func.args[i].type == TOK_ERROR) { break; }
                printf(
                    "%*s@Argument: { %.*s }\n",
                    indent + 2,
                    " ",
                    ast.func.args[i].len,
                    L->source + ast.func.args[i].start
                );
            }

            if (ast.type == AST_FUNCCALL) { break; }

            printf("%*s@Function Body\n", indent + 2, "");
            printASTimpl(*ast.func.body, indent + 4, L);
            break;

        case AST_VALUE:
            printf("%*s@Value: { %d }\n", indent, "", ast.value);
            break;

        case AST_RETURN:
            printf("%*s@Return\n", indent, "");
            printASTimpl(*ast.ret.expr, indent + 2, L);
            break;

        case AST_EXPR:
            printf("%*s@Expr (Token: %c)\n", indent, "", symbols[ast.expr.op]);
            printASTimpl(*ast.expr.lhs, indent + 2, L);
            printASTimpl(*ast.expr.rhs, indent + 2, L);
            break;

        case AST_ERR:
            printf("%*s@Error\n", indent, "");
            break;
        default:
            break;
    }
}

void printAST(ASTNode ast, Aria_Lexer* L) {
    printf("\n=== AST ===\n");
    printASTimpl(ast, 0, L);
}

void nodeFree(ASTNode node) {
    switch(node.type) {
        case AST_BLOCK:
            [[fallthrough]];
        case AST_MODULE:
            for (size_t i = 0; i < node.block->size; i++) {
                nodeFree(*(ASTNode*)bufferGet(node.block, i));
            }
            bufferFree(node.block);
            break;

        case AST_FUNC:
            free(node.func.func_name);
            nodeFree(*node.func.body);
            free(node.func.body);
            break;

        case AST_VALUE:
            [[fallthrough]];
        case AST_ERR:
            break;

        case AST_FUNCCALL:
            free(node.func_call.func_name);
            break;

        case AST_RETURN:
            nodeFree(*node.ret.expr);
            free(node.ret.expr);
            break;

        case AST_EXPR:
            nodeFree(*node.expr.lhs);
            nodeFree(*node.expr.rhs);
            free(node.expr.lhs);
            free(node.expr.rhs);
            break;
        };
}
