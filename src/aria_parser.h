#ifndef ARIA_PARSER_H
#define ARIA_PARSER_H

#define ARIA_BUFFER_IMPL
#include "aria_lexer.h"
#include "aria_buffer.h"

#include <stddef.h>

typedef enum {
    AST_BLOCK,
    AST_FUNC,
    AST_VALUE,
    AST_MODULE,

    AST_ERR,
} ASTType;

typedef struct _ASTNode {
    ASTType type;
    union {
        struct {
            // Aria_Buffer<ASTNode>
            Aria_Buffer* buf;
        } block;

        struct {
            char* func_name;
            Aria_Token args[8];
            struct _ASTNode* body;
        } func;

        int value;
    };
} ASTNode;

ASTNode parseConst(Aria_Lexer* l);
ASTNode parseFor(Aria_Lexer* l);
ASTNode parseIf(Aria_Lexer* l);
ASTNode parsePrint(Aria_Lexer* l);
ASTNode parseReturn(Aria_Lexer* l);
ASTNode parseSwitch(Aria_Lexer* l);
ASTNode parseVar(Aria_Lexer* l);
ASTNode parseIdentifier(Aria_Lexer* l);
ASTNode parseExpression(Aria_Lexer* l);
ASTNode parseClass(Aria_Lexer* l);
ASTNode parseExport(Aria_Lexer* l);
ASTNode parseBlock(Aria_Lexer* l);
ASTNode parseFunc(Aria_Lexer* l);
ASTNode parseImport(Aria_Lexer* l);

ASTNode ariaParse(Aria_Lexer* l);
ASTNode createNode(ASTType type);
void printAST(ASTNode ast, int indent, Aria_Lexer* l);

#endif // ARIA_PARSER_H
