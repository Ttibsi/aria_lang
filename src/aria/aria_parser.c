#include "aria_parser.h"

#include <assert.h>
#include <stdio.h>

#define parsingError(msg) \
    do { \
        fprintf(stderr, msg); \
        assert(0); \
    } while(0)

ASTNode parseConst(Aria_Lexer* L) { assert(0 && "TODO"); }
ASTNode parseFor(Aria_Lexer* L) { assert(0 && "TODO"); }
ASTNode parseIf(Aria_Lexer* L) { assert(0 && "TODO"); }

ASTNode parseReturn(Aria_Lexer* L) {
    ASTNode node = createNode(AST_VALUE);
    advance(L); // go from the return keyword to the value to return

    Aria_Token* tkn = (Aria_Token*)bufferGet(L->tokens, L->buf_index);
    node.value = getTokenNumber(L, tkn);

    advance(L); // consume number
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

    advance(L); // TOK_RIGHT_PAREN
    return node;
}

ASTNode parseIdentifier(Aria_Lexer* L) {
    const Aria_Token* ident = (Aria_Token*)bufferGet(L->tokens, L->buf_index);
    advance(L);

    switch(getCurrTokenType(L)) {
        case TOK_LEFT_PAREN: return parseFuncCall(L); break;
        case  TOK_EQUAL:
            // Probably modifying a variable
            break;
    };
    assert(0 && "TODO");
}

ASTNode parseExpression(Aria_Lexer* L) {
    Aria_Token* tkn = (Aria_Token*)bufferGet(L->tokens, L->buf_index);
    switch (tkn->type) {
        case TOK_CONST: return parseConst(L); break;
        case TOK_FOR: return parseFor(L); break;
        case TOK_IF: return parseIf(L); break;
        case TOK_RETURN: return parseReturn(L); break;
        case TOK_SWITCH: return parseSwitch(L); break;
        case TOK_VAR: return parseVar(L); break;
        case TOK_IDENTIFIER: return parseIdentifier(L); break;
        default: parsingError("Incorrect token found in parseExpression (defauLt)\n"); break;
    };

    parsingError("Incorrect token found in parseExpression\n");
    return (ASTNode){.type = AST_ERR};
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
        ASTNode expr = parseExpression(L);
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

        case AST_ERR:
            return (ASTNode){.type = type};
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

        case AST_FUNC:
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

            printf("%*s@Function Body\n", indent + 2, "");
            printASTimpl(*ast.func.body, indent + 4, L);
            break;

        case AST_VALUE:
            printf("%*s@Value: { %d }\n", indent, "", ast.value);
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
    };
}
