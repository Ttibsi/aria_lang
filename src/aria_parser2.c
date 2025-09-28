#include "aria_parser2.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define parsingError(msg) \
    do { \
        fprintf(stderr, msg); \
        assert(0); \
    } while(0)

ASTNode parseConst(Aria_Lexer* l) { assert(0 && "TODO"); }
ASTNode parseFor(Aria_Lexer* l) { assert(0 && "TODO"); }
ASTNode parseIf(Aria_Lexer* l) { assert(0 && "TODO"); }
ASTNode parsePrint(Aria_Lexer* l) { assert(0 && "TODO"); }

ASTNode parseReturn(Aria_Lexer* l) {
    ASTNode node = createNode(AST_VALUE);
    node.value = getTokenNumber(l, *(Aria_Token*)bufferGet(l->tokens, l->buf_index));
    return node;
}

ASTNode parseSwitch(Aria_Lexer* l) { assert(0 && "TODO"); }
ASTNode parseVar(Aria_Lexer* l) { assert(0 && "TODO"); }
ASTNode parseIdentifier(Aria_Lexer* l) { assert(0 && "TODO"); }

ASTNode parseExpression(Aria_Lexer* l) {
    switch (peek(l)) {
        case TOK_CONST: parseConst(l); break;
        case TOK_FOR: parseFor(l); break;
        case TOK_IF: parseIf(l); break;
        case TOK_PRINT: parsePrint(l); break;
        case TOK_RETURN: parseReturn(l); break;
        case TOK_SWITCH: parseSwitch(l); break;
        case TOK_VAR: parseVar(l); break;
        case TOK_IDENTIFIER: parseIdentifier(l); break;
        default: parsingError("Incorrect token found in parseExpression\n"); break;
    };

    parsingError("Incorrect token found in parseExpression\n");
    return (ASTNode){.type = AST_ERR};
}

ASTNode parseClass(Aria_Lexer* l) { assert(0 && "TODO"); }

ASTNode parseExport(Aria_Lexer* l) { assert(0 && "TODO"); }

ASTNode parseBlock(Aria_Lexer* l) {
    ASTNode node = createNode(AST_BLOCK);

    while (!(
        check(l, TOK_FUNC) ||
        check(l, TOK_CLASS) ||
        check(l, TOK_EXPORT) ||
        check(l, TOK_IMPORT)
    )) {
        ASTNode expr = parseExpression(l);
        bufferInsert(&node.block.buf, (void*)&expr);
    }

    return node;
}

ASTNode parseFunc(Aria_Lexer* l) {
    if (!match(l, TOK_FUNC)) { parsingError("parseFunc called with incorrect token\n"); }
    ASTNode node = createNode(AST_FUNC);

    // function name
    Aria_Token* curr_token = bufferGet(l->tokens, l->buf_index);
    node.func.func_name = malloc(sizeof(char) * curr_token->len + 1);
    memcpy(node.func.func_name, l->source + curr_token->start, curr_token->len);
    node.func.func_name[curr_token->len + 1] = '\0';
    advance(l);

    // arguments
    if (!match(l, TOK_LEFT_PAREN)) { parsingError("Function name not followed by open bracket\n"); }
    int args_idx = 0;
    while (!check(l, TOK_RIGHT_PAREN)) {
        if (!check(l, TOK_IDENTIFIER)) { parsingError("function args contain non-identifiers\n"); }
        if (args_idx >= 8) { parsingError("Function has too many arguments\n"); }

        node.func.args[args_idx] = *(Aria_Token*)bufferGet(l->tokens, l->buf_index);
        args_idx++;
        advance(l);
    }

    // Fill the rest of the 8 arguments
    while (args_idx < 8) {
        node.func.args[args_idx] = (Aria_Token){false, TOK_ERROR, -1. -1};
        args_idx++;
    }

    // body
    ASTNode body = parseBlock(l);
    node.func.body = &body;

    return node;
}

ASTNode parseImport(Aria_Lexer* l) { assert(0 && "TODO"); }

ASTNode ariaParse(Aria_Lexer* l) {
    Aria_Token* curr_token = bufferGet(l->tokens, l->buf_index);
    switch (curr_token->type) {
        case TOK_CLASS:
            return parseClass(l);
        case TOK_EXPORT:
            return parseExport(l);
        case TOK_FUNC:
            return parseFunc(l);
        case TOK_IMPORT:
            return parseImport(l);
        case TOK_ERROR:
        default:
            break;
    };

    return createNode(AST_ERR);
}

ASTNode createNode(ASTType type) {
    switch (type) {
        case AST_BLOCK:
            return (ASTNode){
                .type = type,
                .block = {
                    .buf = bufferCreate(sizeof(ASTNode), 32)
                }
            };

        case AST_FUNC:
            return (ASTNode){
                .type = type,
                .func = {
                    .func_name = NULL,
                    .body = NULL
                }
            };

        case AST_ERR:
            return (ASTNode){.type = type};
    };
}

void printAST(ASTNode ast, size_t indent) {
    switch (ast.type) {
        case AST_BLOCK:
            Aria_Buffer buf = ast.block.buf;
            for (uint32_t i = 0; i < buf.size; i++) {
                printAST(*(ASTNode*)bufferGet(buf, i), indent);
            }
            break;

        case AST_FUNC:
            printf("% *@Function Node: {%s}", indent, ast.func.func_name);

            // Arguments
            for(int i = 0; i < 8; i++) {
                if (ast.func.args[i].type == TOK_ERROR) { break; }
                printf("% *@Argument: {%s}\n", indent + 2, ast.func.args[i]);
            }

            printf("% *@Function Body\n", indent + 2);
            printAST(*ast.func.body, indent + 4);
            break;

        case AST_VALUE:
            printf("% *@Value: {%d}", indent, ast.value);
            break;
        case AST_ERR:
            printf("% *@Error Node", indent);
            break;
        default:
    }
}
