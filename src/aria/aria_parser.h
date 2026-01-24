#ifndef ARIA_PARSER_H
#define ARIA_PARSER_H

#include <stdint.h>
#include <stdlib.h>

#include "aria_lexer.h"

typedef enum {
    AST_ARG,
    AST_BLOCK,
    AST_ERR,
    AST_EXPR,
    AST_IDENT,
    AST_IF,
    AST_FUNC,
    AST_CALL,
    AST_IMPORT,
    AST_MODULE,
    AST_RETURN,
    AST_VALUE,
    AST_VAR,
    AST_STR_LIT,
    AST_NUM_LIT,
    AST_CHAR_LIT,

    NODE_COUNT
} NodeType;

typedef uint8_t binding_t;
constexpr int param_count = 8;

typedef struct _ASTNode {
    NodeType type;

    union {
        // Only used as a pointer
        struct {
            char* name;
            TokenType type;
            TokenType inner_type_1;  // used for LIST and MAP types, ignored otherwise
            TokenType inner_type_2;  // used for MAP type, ignored otherwise
        } arg;

        struct {
            struct _ASTNode* items;
            size_t count;
            size_t capacity;

            char* name;
        } block;

        struct {
            TokenType op;
            struct _ASTNode* lhs;
            struct _ASTNode* rhs;
        } expr;

        struct {
            char* name;
            struct _ASTNode* args[param_count];
            TokenType ret_type;
            struct _ASTNode* body;
        } func;

        struct {
            char* name;
            char* args[param_count];
            TokenType ret_type;
        } funcCall;

        struct {
            struct _ASTNode* cond;
            struct _ASTNode* block;
            struct _ASTNode* elseBlock;
        } If;

        struct {
            bool local_file;
            char* name;
        } import;

        struct {
            struct _ASTNode* expr;
        } ret;

        struct {
            char* name;
            TokenType ret_type;
            struct _ASTNode* value;
        } var;

        int num_literal;
        char* string_literal;
        char char_literal;
        char* identifier;
    };
} ASTNode;

#define ERR_NODE \
    (ASTNode) { .type = AST_ERR }

binding_t prefixBindingPower(const TokenType* tkn);
binding_t infixBindingPower(const TokenType* tkn);
ASTNode parseBlock(AriaLexer* L);
ASTNode parseFunc(AriaLexer* L);
ASTNode parseExpression(AriaLexer* L, const binding_t min_bp, const TokenType endToken);
ASTNode parseFor(AriaLexer* L);
ASTNode parseIdentifier(AriaLexer* L);
ASTNode parseIf(AriaLexer* L);
ASTNode parseImport(AriaLexer* L);
ASTNode parseReturn(AriaLexer* L);
ASTNode parseStatement(AriaLexer* L);
ASTNode parseType(AriaLexer* L);
ASTNode parseVar(AriaLexer* L);
ASTNode ariaCreateNode(const NodeType type);
ASTNode ariaParse(AriaLexer* L, char* mod_name);
void printAst(const ASTNode* root);

#endif  // ARIA_PARSER_H
