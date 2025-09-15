#include "aria_parser2.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

void parsingError(void) { assert(0 && "Parsing error"); }

ASTNode parseClass(Aria_Lexer* l) { assert(0 && "TODO"); }

ASTNode parseExport(Aria_Lexer* l) { assert(0 && "TODO"); }

ASTNode parseFunc(Aria_Lexer* l) {
    if (!match(l, TOK_FUNC)) { parsingError(); }
    ASTNode node = createNode(AST_FUNC);

    // function name
    node.func_name = malloc(sizeof(char) * l->current_token->len + 1);
    node.func_name = strcpy(l->current_token->start, func_name);

    // arguments

    // body

    return node;
}

ASTNode parseImport(Aria_Lexer* l) { assert(0 && "TODO"); }

ASTNode ariaParse(Aria_Lexer* l) {
    // if (!(
    //     check(l, TOK_CLASS) ||
    //     check(l, TOK_EXPORT) ||
    //     check(l, TOK_FUNC) ||
    //     check(l, TOK_IMPORT) ||
    //     check(l, TOK_ERROR) ||
    //     check(l, TOK_EOF)
    // )) {
    //     fprintf(stderr, "Unexpected token at line %d\n", __LINE__);
    //     assert(0);
    // }
    //
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
