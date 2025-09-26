#include "aria_parser2.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void parsingError(const char* msg) { assert(0 && msg); }

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
    Aria_Token* curr_token = bufferGet(l->tokens, l->buf_index);
    node.func.func_name = malloc(sizeof(char) * curr_token->len + 1);
    strcpy(node.func.func_name, l->source + curr_token->start);
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
                    .body = bufferCreate(sizeof(ASTNode), 32)
                }
            };

        case AST_ERR:
            return (ASTNode){.type = type};
    };
}

void printAST(ASTNode ast) {
    switch (ast.type) {
        case AST_BLOCK:
            printf("Block {\n");
            for (int i = 0; i < ast.block.buf.size; i++) {
                ASTNode* node = (ASTNode*)bufferGet(ast.block.buf, i);
                printf("  ");
                printAST(*node);
            }
            printf("}\n");
            break;

        case AST_FUNC:
            printf("Function: %s\n", ast.func.func_name ? ast.func.func_name : "<unnamed>");
            printf("Arguments: ");
            for (int i = 0; i < 8 && ast.func.args[i].type != TOK_EOF; i++) {
                // Assuming args are stored as tokens, print based on token content
                printf("arg%d ", i);
            }
            printf("\n");
            printf("Body {\n");
            for (int i = 0; i < ast.func.body.size; i++) {
                ASTNode* node = (ASTNode*)bufferGet(ast.func.body, i);
                printf("  ");
                printAST(*node);
            }
            printf("}\n");
            break;

        case AST_VALUE:
            printf("Value: %d\n", ast.value);
            break;

        case AST_ERR:
            printf("Error Node\n");
            break;

        default:
            printf("Unknown AST Node Type: %d\n", ast.type);
            break;
    }
}
