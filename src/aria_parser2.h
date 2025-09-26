#ifndef ARIA_PARSER_H
#define ARIA_PARSER_H

#define ARIA_BUFFER_IMPL
#include "aria_lexer.h"
#include "aria_buffer.h"

typedef enum {
    AST_BLOCK,
    AST_FUNC,
    AST_VALUE,

    AST_ERR,
} ASTType;

typedef struct _ASTNode {
    ASTType type;
    union {
        struct {
            Aria_Buffer buf;
        } block;

        struct {
            char* func_name;
            Aria_Token args[8];
            Aria_Buffer body;
        } func;

        int value;
    };
} ASTNode;

ASTNode ariaParse(Aria_Lexer* l);
ASTNode createNode(ASTType type);
void printAST(ASTNode ast);

#endif // ARIA_PARSER_H
