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
 
#define TOKEN_APPEND(tok, begin, size) \
do { \
    Aria_Token t = {.type = tok, .start = begin, .len = size}; \
    skip += (size - 1); \
    aria_lexer_append(&lexer, &t); \
} while (0)

#define NEXT_TOK_IS_EQ(eq_tok, tok) \
do { \
    if (source[i + 1] == '=') { \
        TOKEN_APPEND(eq_tok, i, 2); \
    } else { \
        TOKEN_APPEND(tok, i, 1); \
    } \
} while (0) 

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
    for (size_t i = 0; i < strlen(source); i++) {
        if (skip) { skip--; continue; } 
        if (isspace(source[i])) { continue; } 
 
        // individual characters
        switch (source[i]) {
            // TODO: skip whitespace
            case '.': TOKEN_APPEND(TOK_DOT, i, 1); break;
            case ',': TOKEN_APPEND(TOK_COMMA, i, 1); break; 
            case ';': TOKEN_APPEND(TOK_SEMICOLON, i, 1); break; 
            case '-': TOKEN_APPEND(TOK_MINUS, i, 1); break;
            case '+': TOKEN_APPEND(TOK_PLUS, i, 1); break;
            case '*': TOKEN_APPEND(TOK_STAR, i, 1); break;
            case '/': TOKEN_APPEND(TOK_SLASH, i, 1); break;
            case '{': TOKEN_APPEND(TOK_LEFT_BRACE, i, 1); break;
            case '}': TOKEN_APPEND(TOK_RIGHT_BRACE, i, 1); break;
            case '(': TOKEN_APPEND(TOK_LEFT_PAREN, i, 1); break;
            case ')': TOKEN_APPEND(TOK_RIGHT_PAREN, i, 1); break; 
            case '!': NEXT_TOK_IS_EQ(TOK_BANG_EQUAL, TOK_BANG); break;
            case '=': NEXT_TOK_IS_EQ(TOK_EQUAL_EQUAL, TOK_EQUAL); break;
            case '<': NEXT_TOK_IS_EQ(TOK_LESS_EQUAL, TOK_LESS); break;
            case '>': NEXT_TOK_IS_EQ(TOK_GREATER_EQUAL, TOK_GREATER); break;

            // digraphs 
            // TODO: if not double, add TOK_ERR 
            case '&': if (source[i + 1] == '&') { TOKEN_APPEND(TOK_AND, i, 2); break; }
            case '|': if (source[i + 1] == '|') { TOKEN_APPEND(TOK_OR, i, 2); break; }

            // string literals 
            case '"': {
                const int start = i; 
                int str_lit_len = 0; 
                while (source[i++] != '"') {
                    str_lit_len++; 
                    // TODO: if i >= strlen, TOK_ERR 
                }
                TOKEN_APPEND(TOK_STRING, start, str_lit_len);
            } 
        }

        // keywords 
        for (int j = 0; j < keyword_count; j++) {
            if (strncmp(source + i, keywords[j].kw, keywords[j].len) == 0) {
                TOKEN_APPEND(keywords[j].tok, i, keywords[j].len); 
            }
        }

        // number literals
        int num_len = 0; 
        const int num_start = i; 
        while (isdigit(source[i])) {
            num_len++;
            i++; 
        }
        if (num_len) { TOKEN_APPEND(TOK_NUMBER, num_start, num_len); }

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
            TOKEN_APPEND(TOK_IDENTIFIER, i, identifier_len);
        }
    }

    // EOF     
    TOKEN_APPEND(TOK_EOF, strlen(source), 0); 
    return lexer;
} 

void aria_lexer_append(Aria_Lexer* lexer, Aria_Token* tok) {
    if (lexer->size == lexer->capacity) {
        lexer->data = realloc(lexer->data, sizeof(Aria_Token) * lexer->capacity * 2);
        lexer->capacity *= 2; 
    }
 
    lexer->data[lexer->size] = *tok;  
    lexer->size++; 
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
