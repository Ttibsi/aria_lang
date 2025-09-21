#include "aria_parser2.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void parsingError(const char* msg) { assert(0 && msg); }

ASTNode parseExpression(Aria_Lexer* l) {
    switch (peek(l)) {
        case TOK_CONST: break;
        case TOK_FOR: break;
        case TOK_IF: break;
        case TOK_PRINT: break;
        case TOK_RETURN: break;
        case TOK_SWITCH: break;
        case TOK_VAR: break;
        case TOK_IDENTIFIER: break;
        default: parsingError("Incorrect token found in parseExpression"); break;
    };

    parsingError("Incorrect token found in parseExpression");
    return (ASTNode){.type = AST_ERR};
}

ASTNode parseClass(Aria_Lexer* l) { assert(0 && "TODO"); }

ASTNode parseExport(Aria_Lexer* l) { assert(0 && "TODO"); }

ASTNode parseFunc(Aria_Lexer* l) {
    if (!match(l, TOK_FUNC)) { parsingError("parseFunc called with incorrect token"); }
    ASTNode node = createNode(AST_FUNC);

    // function name
    node.func.func_name = malloc(sizeof(char) * l->current_token.len + 1);
    strcpy(node.func.func_name, l->source + l->current_token.start);
    advance(l);

    // arguments
    if (!match(l, TOK_LEFT_BRACE)) { parsingError("Function name not followed by open bracket"); }
    int args_idx = 0;
    while (!check(l, TOK_RIGHT_BRACE)) {
        if (!check(l, TOK_IDENTIFIER)) { parsingError("function args contain non-identifiers"); }
        if (args_idx >= 8) { parsingError("Function has too many arguments"); }
        //TODO

        args_idx++;
    }

    // body
    while (
        check(l, TOK_FUNC) ||
        check(l, TOK_CLASS) ||
        check(l, TOK_EXPORT) ||
        check(l, TOK_IMPORT)
    ) {
        ASTNode node = parseExpression(l);
        bufferInsert(&node.func.body, (void*)&node);
    }

    return node;
}

ASTNode parseImport(Aria_Lexer* l) { assert(0 && "TODO"); }

ASTNode ariaParse(Aria_Lexer* l) {
    switch (l->current_token.type) {
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
                    .body = bufferCreate(sizeof(ASTNode), 32)
                }
            };

        case AST_ERR:
            return (ASTNode){.type = type};
    };
}

void printAST(ASTNode ast) {
    assert(0 && "TODO");
}
