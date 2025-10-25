#ifndef ARIA_PARSER_H
#define ARIA_PARSER_H

#include "aria_lexer.h"
#include "aria_buffer.h"

constexpr int param_count = 8;

typedef enum {
    AST_BLOCK,
    AST_FUNC,
    AST_VALUE,
    AST_MODULE,
    AST_FUNCCALL,
    AST_RETURN,
    AST_EXPR,

    AST_ERR,
} ASTType;

typedef struct _ASTNode {
    ASTType type;
    union {
        struct {
            char* func_name;
            Aria_Token args[param_count];
            struct _ASTNode* body;
        } func;

        struct {
            char* func_name;
            Aria_Token* param_list[param_count];
        } func_call;

        struct {
            struct _ASTNode* expr;
        } ret;

        struct {
            TokenType op;
            struct _ASTNode* lhs;
            struct _ASTNode* rhs;
        } expr;

        int value;

        // Aria_Buffer<ASTNode>
        Aria_Buffer* block;
    };
} ASTNode;

typedef uint8_t binding_t;

binding_t prefixBindingPower(const TokenType* tkn);
binding_t infixBindingPower(const TokenType* tkn);
ASTNode parseConst(Aria_Lexer* L);
ASTNode parseFor(Aria_Lexer* L);
ASTNode parseIf(Aria_Lexer* L);
ASTNode parseReturn(Aria_Lexer* L);
ASTNode parseSwitch(Aria_Lexer* L);
ASTNode parseVar(Aria_Lexer* L);
ASTNode parseFuncCall(Aria_Lexer* L);
ASTNode parseIdentifier(Aria_Lexer* L);
ASTNode parseExpression(Aria_Lexer* L, binding_t min_bp);
ASTNode parseClass(Aria_Lexer* L);
ASTNode parseExport(Aria_Lexer* L);
ASTNode parseImport(Aria_Lexer* l);
ASTNode parseBlock(Aria_Lexer* L);
ASTNode parseFunc(Aria_Lexer* L);
ASTNode parseStatement(Aria_Lexer* L);
ASTNode ariaParse(Aria_Lexer* L);
ASTNode createNode(ASTType type);
void printAST(ASTNode ast, Aria_Lexer* L);
void nodeFree(ASTNode node);

#endif // ARIA_PARSER_H
