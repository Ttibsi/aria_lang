#include "aria_parser.h"

#include <string.h>
#include <stdio.h>

Aria_ASTNode* parseBinaryExpr(Aria_Lexer* l) {
    if (match(l, TOK_PLUS) || match(l, TOK_STAR)) {
        Aria_ASTNode* node = malloc(sizeof(Aria_ASTNode));
        node->type = AST_BINARY;
        node->as.binary.op = l->current_token.type; // Previous token that was matched
        node->as.binary.lhs = NULL; // Would need proper operand parsing
        node->as.binary.rhs = NULL; // Would need proper operand parsing
        return node;
    }
    return NULL;
}

Aria_ASTNode* parsePrimary(Aria_Lexer* l, Aria_Token* tok) {
    Aria_ASTNode* node = malloc(sizeof(Aria_ASTNode));
    switch (tok->type) {
        case TOK_TRUE:
        case TOK_FALSE:
            node->type = AST_BOOL;
            node->as.literal.val.boolean = (tok->len == 4);
            break; 

        case TOK_STRING:
            node->type = AST_STRING;
            node->as.literal.val.text = (char*)malloc(sizeof(char) * tok->len + 1);
            memcpy(node->as.literal.val.text, &l->source[tok->start], tok->len);
            node->as.literal.val.text[tok->len] = '\0'; // NOTE: potential off by one?
            break; 

        case TOK_IDENTIFIER: 
            node->type = AST_IDENTIFIER; 
            node->as.literal.val.text = (char*)malloc(sizeof(char) * tok->len + 1); 
            memcpy(node->as.literal.val.text, &l->source[tok->start], tok->len); 
            node->as.literal.val.text[tok->len] = '\0';   // NOTE: potential off by one
            break; 

        case TOK_NUMBER:
            node->type = AST_NUMBER;
            char* endptr = NULL; 
            node->as.literal.val.number = strtof(&l->source[tok->start], &endptr);
            break; 

        default:
            break; 
    }

    return node; 
} 

Aria_ASTNode* parseExpression(Aria_Lexer* l) {
    return parseBinaryExpr(l);
}

void printAST(Aria_ASTNode* node, int depth) {
    if (!node) return;

    for (int i = 0; i < depth; i++) printf("  ");
    printf("Node type: %d\n", node->type);

    if (node->type == AST_BINARY) {
        printAST(node->as.binary.lhs, depth + 1);
        printAST(node->as.binary.rhs, depth + 1);
    }
}
