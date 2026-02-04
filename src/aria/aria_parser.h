#ifndef ARIA_PARSER_H
#define ARIA_PARSER_H

#include <stdint.h>
#include <stdlib.h>

#include "aria_lexer.h"

typedef enum {
    AST_ARG,
    AST_ASSIGN,
    AST_BLOCK,
    AST_CALL,
    AST_CHAR_LIT,
    AST_ERR,
    AST_EXPR,
    AST_FOR,
    AST_FOREACH,
    AST_FUNC,
    AST_IDENT,
    AST_IF,
    AST_IMPORT,
    AST_METHOD_CALL,
    AST_MODULE,
    AST_NUM_LIT,
    AST_RETURN,
    AST_STR_LIT,
    AST_TYPE,
    AST_VAR,

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
            struct _ASTNode* ident;
            struct _ASTNode* object_ident;
            struct _ASTNode* expr;
        } assign;

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
            struct _ASTNode* var;
            struct _ASTNode* start;
            struct _ASTNode* stop;
            struct _ASTNode* step;
            struct _ASTNode* block;
        } For;

        struct {
            struct _ASTNode* first_var;
            struct _ASTNode* sec_var;
            struct _ASTNode* container;
            struct _ASTNode* block;
        } ForEach;

        struct {
            char* name;
            struct _ASTNode* args[param_count];
            TokenType ret_type;
            struct _ASTNode* body;
        } func;

        struct {
            char* name;
            char* args[param_count];
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
            char* object;
            struct _ASTNode* method;  // ptr to funcCall object
        } methodCall;

        struct {
            struct _ASTNode* expr;
        } ret;

        struct {
            char* name;
            struct {
                struct _ASTNode* items;
                size_t count;
                size_t capacity;
            } Vars;

            struct {
                struct _ASTNode* items;
                size_t count;
                size_t capacity;
            } Methods;
        } Type;

        struct {
            char* name;
            TokenType ret_type;
            struct _ASTNode* value;
        } var;

        int num_literal;
        char* string_literal;
        char char_literal;
        char* identifier;
        bool bool_literal;
    };
} ASTNode;

#define ERR_NODE \
    (ASTNode) { .type = AST_ERR }

binding_t prefixBindingPower(const TokenType* tkn);
binding_t infixBindingPower(const TokenType* tkn);
char* getStringName(AriaLexer* L);
ASTNode* parseArg(AriaLexer* L);
ASTNode parseAssignment(AriaLexer* L);
ASTNode parseBlock(AriaLexer* L);
ASTNode parseFunc(AriaLexer* L);
ASTNode parseFuncCall(AriaLexer* L);
ASTNode parseExpression(AriaLexer* L, const binding_t min_bp);
ASTNode parseFor(AriaLexer* L);
ASTNode parseForEach(AriaLexer* L);
ASTNode parseIdentifier(AriaLexer* L);
ASTNode parseIf(AriaLexer* L);
ASTNode parseImport(AriaLexer* L);
ASTNode parseReturn(AriaLexer* L);
ASTNode parseMethodCall(AriaLexer* L);
ASTNode parseMethodCallOrAttr(AriaLexer* L);
ASTNode parseStatement(AriaLexer* L);
ASTNode parseType(AriaLexer* L);
ASTNode parseVar(AriaLexer* L);
ASTNode ariaCreateNode(const NodeType type);
ASTNode ariaParse(AriaLexer* L, char* mod_name);
void printAst(const ASTNode* root);

#endif  // ARIA_PARSER_H
