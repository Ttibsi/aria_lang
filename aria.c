#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "aria.h"
#define ARIA_DEBUG 1 

// Lexing
char peek(Aria_Lexer* l) {
    return l->source[l->pc];
}

char peekNext(Aria_Lexer* l) {
    return l->source[l->pc + 1];
}

char advanceChar(Aria_Lexer* l) {
    return l->source[l->pc++];
}

void skipWhitespace(Aria_Lexer* l) {
    while (isspace(peek(l))) {
        advanceChar(l);
    }
}

Aria_Token makeToken(Aria_Lexer* l, TokenType type, int start, int length) {
    return (Aria_Token){
        .valid = true,
        .type = type,
        .start = start,
        .len = length
    };
}

Aria_Token scanEqualVariant(Aria_Lexer* l, TokenType single, TokenType equal) {
    int start = l->pc - 1;
    if (peek(l) == '=') {
        advanceChar(l);
        return makeToken(l, equal, start, 2);
    }
    return makeToken(l, single, start, 1);
}

Aria_Token scanStringLiteral(Aria_Lexer* l) {
    int start = l->pc - 1;
    int length = 0;
    
    while (peek(l) != '"' && peek(l) != '\0') {
        advanceChar(l);
        length++;
    }
    
    if (peek(l) == '\0') {
        // TODO: Handle error case
        return makeToken(l, TOK_EOF, start, 0);
    }
    
    advanceChar(l); // consume closing quote
    return makeToken(l, TOK_STRING, start, length + 2);
}

Aria_Token scanNumber(Aria_Lexer* l) {
    int start = l->pc - 1;
    int length = 1;
    
    while (isdigit(peek(l))) {
        advanceChar(l);
        length++;
    }
    
    return makeToken(l, TOK_NUMBER, start, length);
}

Aria_Token scanIdentifier(Aria_Lexer* l) {
    int start = l->pc - 1;
    int length = 1;
    
    while (isalnum(peek(l)) || peek(l) == '_') {
        advanceChar(l);
        length++;
    }
    
    // Check if it's a keyword
    for (int i = 0; i < keyword_count; i++) {
        if (keywords[i].len == length && 
            strncmp(l->source + start, keywords[i].kw, length) == 0) {
            return makeToken(l, keywords[i].tok, start, length);
        }
    }
    
    return makeToken(l, TOK_IDENTIFIER, start, length);
}

Aria_Token scanToken(Aria_Lexer* l) {
    skipWhitespace(l);
    
    char c = advanceChar(l);
    int start = l->pc - 1;
    
    if (c == '\0') {
        return makeToken(l, TOK_EOF, start, 0);
    }
    
    switch (c) {
        case '.': return makeToken(l, TOK_DOT, start, 1);
        case ',': return makeToken(l, TOK_COMMA, start, 1);
        case ';': return makeToken(l, TOK_SEMICOLON, start, 1);
        case '-': return makeToken(l, TOK_MINUS, start, 1);
        case '+': return makeToken(l, TOK_PLUS, start, 1);
        case '*': return makeToken(l, TOK_STAR, start, 1);
        case '/': return makeToken(l, TOK_SLASH, start, 1);
        case '{': return makeToken(l, TOK_LEFT_BRACE, start, 1);
        case '}': return makeToken(l, TOK_RIGHT_BRACE, start, 1);
        case '(': return makeToken(l, TOK_LEFT_PAREN, start, 1);
        case ')': return makeToken(l, TOK_RIGHT_PAREN, start, 1);
        case '!': return scanEqualVariant(l, TOK_BANG, TOK_BANG_EQUAL);
        case '=': return scanEqualVariant(l, TOK_EQUAL, TOK_EQUAL_EQUAL);
        case '<': return scanEqualVariant(l, TOK_LESS, TOK_LESS_EQUAL);
        case '>': return scanEqualVariant(l, TOK_GREATER, TOK_GREATER_EQUAL);
        case '&':
            if (peek(l) == '&') {
                advanceChar(l);
                return makeToken(l, TOK_AND, start, 2);
            }
            break;
        case '|':
            if (peek(l) == '|') {
                advanceChar(l);
                return makeToken(l, TOK_OR, start, 2);
            }
            break;
        case '"': return scanStringLiteral(l);
    }
    
    if (isdigit(c)) {
        return scanNumber(l);
    }
    
    if (isalpha(c) || c == '_') {
        return scanIdentifier(l);
    }
    
    // TODO: Handle error case
    return makeToken(l, TOK_EOF, start, 0);
}

void advance(Aria_Lexer* l) {
    l->current_token = scanToken(l);
}

bool check(Aria_Lexer* l, TokenType type) {
    return l->current_token.type == type;
}

bool match(Aria_Lexer* l, TokenType type) {
    if (check(l, type)) {
        advance(l);
        return true;
    }
    return false;
}

/// Parsing
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

// TODO: Implement a hash table here from crafting interpreters for variable assignment

/// VM Interface
int aria_interpret(Aria_VM* vm, const char* name, const char* src) {
    Aria_Lexer lexer = {src, 0, {false, TOK_EOF, 0, 0}};

#ifdef ARIA_DEBUG
    printf("=== TOKENS ===\n"); 
    Aria_Token token;
    do {
        token = scanToken(&lexer);
        printf("Token: %d, start: %d, len: %d\n", token.type, token.start, token.len);
    } while (token.type != TOK_EOF);
    
    // Reset lexer for parsing
    lexer.pc = 0;
#endif

    // Initialize with first token
    advance(&lexer);
    
    // Parse into AST and store in VM
    vm->ast_root = parseExpression(&lexer);
    
#ifdef ARIA_DEBUG
    printf("\n=== AST ===\n"); 
    printAST(vm->ast_root, 0);
#endif

    // eval here
    // cleanup here

    return 0;
}

Aria_VM aria_vm_init(void) {
    return (Aria_VM){0};
}

void aria_vm_destroy(Aria_VM* vm) {
    // VM cleanup will be implemented later
}
