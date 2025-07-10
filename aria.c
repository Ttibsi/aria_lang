#include "aria.h"

#include <ctype.h> 
#include <stddef.h> 
#include <string.h> 

// TODO: Investigate using an arena for memory allocation instead
 
Aria_VM aria_vm_init() {
    return (Aria_VM){
        .lexer = NULL
    };
}

void aria_vm_destroy(Aria_VM* aria_vm) { 
}

int aria_interpret(Aria_VM* aria_vm, const char* module, const char* source) {
    Aria_Lexer temp = aria_tokenize(aria_vm, module, source);
    aria_vm->lexer = malloc(sizeof(Aria_Lexer));
    memcpy(aria_vm->lexer, &temp, sizeof(Aria_Lexer)); 
 
#if ARIA_DEBUG == 1
    print_tokens(aria_vm->lexer); 
#endif 
    
    Aria_Parser parser = aria_parse(aria_vm);
    aria_vm->parser = malloc(sizeof(Aria_Parser));
    memcpy(aria_vm->parser, &parser, sizeof(Aria_Parser)); 
}

int aria_lexer_append(Aria_Lexer* l, const TokenType tok, const int begin, const int size) {
    Aria_Token t = {.type = tok, .start = begin, .len = size}; \
    if (l->size == l->capacity) {
        l->data = realloc(l->data, sizeof(Aria_Token) * l->capacity * 2);
        l->capacity *= 2; 
    }
 
    l->data[l->size] = t;  
    l->size++; 
    return size - 1;
} 

int next_tok_is_eq(Aria_Lexer* lexer, const char* text, TokenType tok, const int pos) {
    if (text + pos + 1 == '=') {
        return aria_lexer_append(lexer, tok + 1, pos, 2);
    } else { 
        return aria_lexer_append(lexer, tok, pos, 1); 
    }
} 

Aria_Lexer aria_tokenize(Aria_VM* vm, const char* module, const char* source) {
    Aria_Lexer lexer = {
        .data = malloc(32 * sizeof(Aria_Token)),
        .size = 0,
        .capacity = 32,
        .source = source,
        .module = module,
        .error = false 
    };

    int skip = 0; 
    for (size_t i = 0; i <= strlen(source); i++) {
        if (skip) { skip--; continue; } 
        if (isspace(source[i])) { continue; } 
 
        // individual characters
        switch (source[i]) {
            // TODO: skip whitespace
            case '.': skip += aria_lexer_append(&lexer, TOK_DOT, i, 1); continue;
            case ',': skip += aria_lexer_append(&lexer, TOK_COMMA, i, 1); continue; 
            case ';': skip += aria_lexer_append(&lexer, TOK_SEMICOLON, i, 1); continue; 
            case '-': skip += aria_lexer_append(&lexer, TOK_MINUS, i, 1); continue;
            case '+': skip += aria_lexer_append(&lexer, TOK_PLUS, i, 1); continue;
            case '*': skip += aria_lexer_append(&lexer, TOK_STAR, i, 1); continue;
            case '/': skip += aria_lexer_append(&lexer, TOK_SLASH, i, 1); continue;
            case '{': skip += aria_lexer_append(&lexer, TOK_LEFT_BRACE, i, 1); continue;
            case '}': skip += aria_lexer_append(&lexer, TOK_RIGHT_BRACE, i, 1); continue;
            case '(': skip += aria_lexer_append(&lexer, TOK_LEFT_PAREN, i, 1); continue;
            case ')': skip += aria_lexer_append(&lexer, TOK_RIGHT_PAREN, i, 1); continue; 
            case '!': skip += next_tok_is_eq(&lexer, source, TOK_BANG, i); continue;
            case '=': skip += next_tok_is_eq(&lexer, source, TOK_EQUAL, i); continue;
            case '<': skip += next_tok_is_eq(&lexer, source, TOK_LESS, i); continue;
            case '>': skip += next_tok_is_eq(&lexer, source, TOK_GREATER, i); continue;

            // digraphs 
            // TODO: if not double, add TOK_ERR 
            case '&': if (source[i + 1] == '&') { skip += aria_lexer_append(&lexer, TOK_AND, i, 2); continue; }
            case '|': if (source[i + 1] == '|') { skip += aria_lexer_append(&lexer, TOK_OR, i, 2); continue; }

            // string literals 
            case '"': {
                const int start = i; 
                int str_lit_len = 0; 
                while (source[i++] != '"') {
                    str_lit_len++; 
                    // TODO: if i >= strlen, TOK_ERR 
                }
                skip += aria_lexer_append(&lexer, TOK_STRING, start, str_lit_len);
            } continue;
        }

        bool found = false; 

        // keywords  
        for (int j = 0; j < keyword_count; j++) {
            if (strncmp(source + i, keywords[j].kw, keywords[j].len) == 0) {
                found = true; 
                skip += aria_lexer_append(&lexer, keywords[j].tok, i, keywords[j].len); 
                break;
            }
        }        
        if (found) { continue; } 

        // number literals
        int num_len = 0; 
        const int num_start = i; 
        while (isdigit(source[i])) {
            num_len++;
            i++; 
        }
        if (num_len) {
            skip += aria_lexer_append(&lexer, TOK_NUMBER, num_start, num_len) - 1;
            if (skip < 0) { skip = 0; } 
            i--; 
            continue; 
        }

        // identifiers: [a-zA-Z][a-zA_Z0-9_]+
        if (('a' <= source[i] && source[i] <= 'z') || ('A' <= source[i] && source[i] <= 'Z')) {
            int identifier_len = 1;
            while (i + identifier_len < strlen(source) && 
                   (('a' <= source[i + identifier_len] && source[i + identifier_len] <= 'z') ||
                    ('A' <= source[i + identifier_len] && source[i + identifier_len] <= 'Z') ||
                    ('0' <= source[i + identifier_len] && source[i + identifier_len] <= '9') ||
                    source[i + identifier_len] == '_')) {
                identifier_len++;
            }
            skip += aria_lexer_append(&lexer, TOK_IDENTIFIER, i, identifier_len); 
        }
    }

    // EOF     
    aria_lexer_append(&lexer, TOK_EOF, strlen(source), 0); 
    return lexer;
} 

void print_tokens(Aria_Lexer* lexer) {
    printf("=== TOKENS ===\n");
 
    for (int i = 0; i < lexer->size; i++) {
        Aria_Token* tok = &lexer->data[i]; 
        printf(
            "Token: %2d, pos: %d, size: %d - %.*s\n",
            tok->type,
            tok->start,
            tok->len,
            tok->len,  // length of format specifier to print letters
            lexer->source + tok->start
        );
    } 
}

// https://vey.ie/2018/10/04/RecursiveDescent.html
// https://ruairidh.dev/build-your-own-ast/
Aria_Parser aria_parse(Aria_VM* aria_vm) {
    Aria_Parser parser = {
        .data = malloc(32 * sizeof(Aria_AstNode)),
        .size = 0,
        .capacity = 32,
    };

    for (int i = 0; i < aria_vm->lexer->size; i++) {
        Aria_Token* current = &aria_vm->lexer->data[i];
        aria_parser_append(&parser, aria_parse_tok(aria_vm->lexer, i, current));

    }
    return parser;
}

Aria_Token next_tok(Aria_Lexer* l, int pos) {
    return l->data[pos + 1];
}

Aria_AstNode aria_parse_tok(Aria_Lexer* l, int pos, Aria_Token* cur) {

    switch (cur->type) {
        case TOK_LEFT_PAREN:
            Aria_AstNode node = (Aria_AstNode){
                .type = AST_BRACKET_BLOCK,
                .is.block = {
                    .data = malloc(32 * sizeof(struct block)),
                    .size = 0,
                    .capacity = 32,
                }
            };

            while (next_tok(l, pos).type != TOK_RIGHT_PAREN) {
                Aria_Token temp = next_tok(l, pos + 1);
                aria_parser_block_append(&node.is.block, aria_parse_tok(l, pos + 1, &temp));
            }

            return node;

        case TOK_RIGHT_PAREN:
            return (Aria_AstNode){
                .type = AST_NULL,
                .is.none = {}
            };

        case TOK_LEFT_BRACE:
        case TOK_RIGHT_BRACE:
        case TOK_COMMA:
        case TOK_DOT:
        case TOK_SEMICOLON:
        case TOK_MINUS:
        case TOK_PLUS:
        case TOK_SLASH:
        case TOK_STAR:
        case TOK_BANG:
        case TOK_BANG_EQUAL:
        case TOK_EQUAL:
        case TOK_EQUAL_EQUAL:
        case TOK_GREATER:
        case TOK_GREATER_EQUAL:
        case TOK_LESS:
        case TOK_LESS_EQUAL:
        case TOK_AND:
        case TOK_OR:
        case TOK_CASE:
        case TOK_CLASS:
        case TOK_CONST:
        case TOK_DEFAULT:
        case TOK_ELSE:
        case TOK_FALSE:
        case TOK_FOR:
        case TOK_FUNC:
        case TOK_IF:
        case TOK_RETURN:
        case TOK_STATIC:
        case TOK_SWITCH:
        case TOK_TRUE:
        case TOK_VAR:
        case TOK_IDENTIFIER:
            return (Aria_AstNode){
                .type = AST_IDENTIFIER,
                .is.literal = { .token = cur }
            };
        case TOK_STRING:
            return (Aria_AstNode){
                .type = AST_STR_LITERAL,
                .is.literal = { .token = cur }
            };

        case TOK_NUMBER:
            return (Aria_AstNode){
                .type = AST_NUMERIC,
                .is.literal = { .token = cur }
            };
        case TOK_ERROR:
        case TOK_EOF:
          break;
        };
}

void aria_parser_append(Aria_Parser* p, const Aria_AstNode node) {
    if (p->size == p->capacity) {
        p->data = realloc(p->data, sizeof(Aria_Parser) * p->capacity * 2);
        p->capacity *= 2; 
    }
 
    p->data[p->size] = node;  
    p->size++; 
}

void aria_parser_block_append(struct block* b, const Aria_AstNode node) {
    if (b->size == b->capacity) {
        b->data = realloc(b->data, sizeof(struct block) * b->capacity * 2);
        b->capacity *= 2; 
    }
 
    b->data[b->size] = node;  
    b->size++; 
}
