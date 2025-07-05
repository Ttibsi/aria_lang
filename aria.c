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
}

#define NEXT_TOK_IS_EQ(eq_tok, tok) \
do { \
    if (source[i + 1] == '=') { \
        TOKEN_APPEND(eq_tok, i, 2); \
    } else { \
        TOKEN_APPEND(tok, i, 1); \
    } \
} while (0) 

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
