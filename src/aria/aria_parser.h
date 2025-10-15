#ifndef ARIA_PARSER_H
#define ARIA_PARSER_H

#include "aria_lexer.h"
#include "aria_buffer.h"

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

ASTNode parseConst(Aria_Lexer* L);
ASTNode parseFor(Aria_Lexer* L);
ASTNode parseIf(Aria_Lexer* L);
ASTNode parsePrint(Aria_Lexer* L);
ASTNode parseReturn(Aria_Lexer* L);
ASTNode parseSwitch(Aria_Lexer* L);
ASTNode parseVar(Aria_Lexer* L);
ASTNode parseIdentifier(Aria_Lexer* L);
ASTNode parseExpression(Aria_Lexer* L);
ASTNode parseClass(Aria_Lexer* L);
ASTNode parseExport(Aria_Lexer* L);
ASTNode parseImport(Aria_Lexer* l);
ASTNode parseBlock(Aria_Lexer* L);
ASTNode parseFunc(Aria_Lexer* L);
ASTNode ariaParse(Aria_Lexer* L);
ASTNode createNode(ASTType type);
void printAST(ASTNode ast, Aria_Lexer* L);

#endif // ARIA_PARSER_H
