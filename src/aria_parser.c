#include "aria_parser.h"
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
    advance(l); // go from the return keyword to the value to return
    node.value = getTokenNumber(l, *(Aria_Token*)bufferGet(l->tokens, l->buf_index));
    advance(l); // consume number
    advance(l); // consume semicolon
    return node;
}

ASTNode parseSwitch(Aria_Lexer* l) { assert(0 && "TODO"); }
ASTNode parseVar(Aria_Lexer* l) { assert(0 && "TODO"); }
ASTNode parseIdentifier(Aria_Lexer* l) { assert(0 && "TODO"); }

ASTNode parseExpression(Aria_Lexer* l) {
    switch (((Aria_Token*)bufferGet(l->tokens, l->buf_index))->type) {
        case TOK_CONST: return parseConst(l); break;
        case TOK_FOR: return parseFor(l); break;
        case TOK_IF: return parseIf(l); break;
        case TOK_PRINT: return parsePrint(l); break;
        case TOK_RETURN: return parseReturn(l); break;
        case TOK_SWITCH: return parseSwitch(l); break;
        case TOK_VAR: return parseVar(l); break;
        case TOK_IDENTIFIER: return parseIdentifier(l); break;
        default: parsingError("Incorrect token found in parseExpression (default)\n"); break;
    };

    parsingError("Incorrect token found in parseExpression\n");
    return (ASTNode){.type = AST_ERR};
}

ASTNode parseClass(Aria_Lexer* l) { assert(0 && "TODO"); }

ASTNode parseExport(Aria_Lexer* l) { assert(0 && "TODO"); }

ASTNode parseBlock(Aria_Lexer* l) {
    ASTNode node = createNode(AST_BLOCK);
    advance(l); // TOK_LEFT_BRACE

    while (!(
        check(l, TOK_FUNC) ||
        check(l, TOK_CLASS) ||
        check(l, TOK_EXPORT) ||
        check(l, TOK_IMPORT)
    )) {
        ASTNode expr = parseExpression(l);
        bufferInsert(node.block.buf, (void*)&expr);

        if (check(l, TOK_RIGHT_BRACE)) { break; }
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
    node.func.func_name[curr_token->len] = '\0';
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
        if (check(l, TOK_COMMA)) { advance(l); } // Skip commas
    }

    advance(l); // TOK_RIGHT_PAREN

    // Fill the rest of the 8 arguments
    while (args_idx < 8) {
        node.func.args[args_idx] = (Aria_Token){false, TOK_ERROR, -1, -1};
        args_idx++;
    }

    // body
    ASTNode* body = malloc(sizeof(ASTNode));
    *body = parseBlock(l);
    node.func.body = body;

    return node;
}

ASTNode parseImport(Aria_Lexer* l) { assert(0 && "TODO"); }

ASTNode ariaParse(Aria_Lexer* l) {
    ASTNode module = createNode(AST_MODULE);

    while (!check(l, TOK_EOF)) {
        ASTNode inner = (ASTNode){.type = AST_ERR};

        switch (getCurrTokenType(l)) {
            case TOK_CLASS:
                inner = parseClass(l);
                break;
            case TOK_EXPORT:
                inner = parseExport(l);
                break;
            case TOK_FUNC:
                inner = parseFunc(l);
                advance(l); // TOK_RIGHT_BRACE
                break;
            case TOK_IMPORT:
                inner = parseImport(l);
                break;
            case TOK_ERROR:
            default:
                break;
        };

        bufferInsert(module.block.buf, &inner);
    }

    return module;
}

ASTNode createNode(ASTType type) {
    switch (type) {
        case AST_BLOCK: case AST_MODULE:
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

        case AST_VALUE:
            return (ASTNode){
                .type = type,
                .value = 0
            };

        case AST_ERR:
            return (ASTNode){.type = type};
    };

    return (ASTNode){.type = -1};
}

void printAST(ASTNode ast, int indent, Aria_Lexer* l) {
    switch (ast.type) {
        case AST_BLOCK: case AST_MODULE:
            Aria_Buffer* buf = ast.block.buf;
            for (uint32_t i = 0; i < buf->size; i++) {
                printAST(*(ASTNode*)bufferGet(buf, i), indent, l);
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
                    l->source + ast.func.args[i].start
                );
            }

            printf("%*s@Function Body\n", indent + 2, "");
            printAST(*ast.func.body, indent + 4, l);
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
