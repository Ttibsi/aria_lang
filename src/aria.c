#include "aria.h"

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
IMPORT "file.ext"
IMPORT module

; a comment

FUNC main(argc NUM, argv LIST[STR]) NUM
    VAR x NUM = 5
    VAR y NUM = 10

    IF y < x THEN
        ...
    ELSE IF x == y THEN
        ...
    ELSE
        ...
    END

    ; `STEP int` is optional
    FOR i = 1 TO x STEP 2 THEN
        ...
    END

    ; idx and elem for a list, key and value for a map
    FOREACH idx, elem IN container THEN
        ...
    END

    func_call(param)  ; function call

    RET 0
END
*/

///// Type Aliases
typedef uint64_t Value;

///// Macros
#define DA(T, name)   \
    struct name {     \
        T* items;     \
        int count;    \
        int capacity; \
    } name

#define max(a, b)               \
    ({                          \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a > _b ? _a : _b;      \
    })

#define da_append(ptr, item)                                       \
    do {                                                           \
        if ((ptr)->count == (ptr)->capacity) {                     \
            (ptr)->capacity = max(64, (ptr)->capacity * 2);        \
            (ptr)->items = realloc((ptr)->items, (ptr)->capacity); \
        }                                                          \
        (ptr)->items[(ptr)->count++] = item;                       \
    } while (0)

/// Structs and Enums
typedef struct {
    char* data;
    size_t len;

    uint32_t hash;
} String;

DA(String, Strings);

typedef struct {
    String key;
    Value value;  // Should this be something else?
} HashTableElem;

typedef struct {
    DA(HashTableElem, buckets)[16];
    // struct buckets {
    //     HashTableElem* items;
    //     int count;
    //     int capacity;
    // } buckets[16];

} Table;

typedef enum {
    // Single-character tokens.
    TOK_LEFT_PAREN,      // 0
    TOK_RIGHT_PAREN,     // 1
    TOK_LEFT_SQUACKET,   // 2
    TOK_RIGHT_SQUACKET,  // 3
    TOK_COMMA,           // 4
    TOK_DOT,             // 5
    TOK_SEMICOLON,       // 6
    TOK_COLON,           // 7
    TOK_MINUS,           // 8
    TOK_PLUS,            // 9
    TOK_SLASH,           // 10
    TOK_STAR,            // 11

    // One or two character tokens.
    TOK_BANG,           // 12
    TOK_BANG_EQUAL,     // 13
    TOK_EQUAL,          // 14
    TOK_EQUAL_EQUAL,    // 15
    TOK_GREATER,        // 16
    TOK_GREATER_EQUAL,  // 17
    TOK_LESS,           // 18
    TOK_LESS_EQUAL,     // 19

    // digraphs
    TOK_AND,       // 20
    TOK_OR,        // 21
    TOK_ELLIPSIS,  // 22

    // Keywords.
    TOK_BOOL,     // 23
    TOK_CHAR,     // 24
    TOK_ELSE,     // 25
    TOK_END,      // 26
    TOK_FALSE,    // 27
    TOK_FOR,      // 28
    TOK_FOREACH,  // 29
    TOK_FUNC,     // 30
    TOK_IF,       // 31
    TOK_IMPORT,   // 32
    TOK_IN,       // 33
    TOK_LIST,     // 34
    TOK_MAP,      // 35
    TOK_NIL,      // 36
    TOK_NUM,      // 37
    TOK_RET,      // 38
    TOK_STEP,     // 39
    TOK_STR,      // 40
    TOK_THEN,     // 41
    TOK_TO,       // 42
    TOK_TRUE,     // 43
    TOK_TYPE,     // 44
    TOK_VAR,      // 45

    // Literals.
    TOK_IDENTIFIER,  // 46
    TOK_STRING_LIT,  // 47
    TOK_CHAR_LIT,    // 48
    TOK_NUM_LIT,     // 49

    TOK_ERROR,  // 50
    TOK_EOF,    // 51
    TOK_COUNT
} TokType;

typedef struct {
    TokType type;
    int start;
    int length;
    int line;
} Token;

typedef struct {
    const char* kw;
    int len;
    TokType tok;
} Keyword;

// clang-format off
static const Keyword keywords[] = {
    {"BOOL",    4, TOK_BOOL    }, // 23
    {"CHAR",    4, TOK_CHAR    }, // 24
    {"ELSE",    4, TOK_ELSE    }, // 25
    {"END",     3, TOK_END     }, // 26
    {"FALSE",   5, TOK_FALSE   }, // 27
    {"FOR",     3, TOK_FOR     }, // 28
    {"FOREACH", 7, TOK_FOREACH }, // 29
    {"FUNC",    4, TOK_FUNC    }, // 30
    {"IF",      2, TOK_IF      }, // 31
    {"IMPORT",  6, TOK_IMPORT  }, // 32
    {"IN",      2, TOK_IN      }, // 33
    {"LIST",    4, TOK_LIST    }, // 34
    {"MAP",     3, TOK_MAP     }, // 35
    {"NIL",     3, TOK_NIL     }, // 36
    {"NUM",     3, TOK_NUM     }, // 37
    {"RET",     3, TOK_RET     }, // 38
    {"STEP",    4, TOK_STEP    }, // 39
    {"STR",     3, TOK_STR     }, // 40
    {"THEN",    4, TOK_THEN    }, // 41
    {"TO",      2, TOK_TO      }, // 42
    {"TRUE",    4, TOK_TRUE    }, // 43
    {"TYPE",    4, TOK_TYPE    }, // 44
    {"VAR",     3, TOK_VAR     }, // 45
};
static const int keyword_count = sizeof(keywords) / sizeof(keywords[0]);
// clang-format on

typedef struct {
    Token prev;
    Token current;
    String input;
    Status status;  // This may not be needed? ParserStatus instead? Status was repurposed for
                    // public error reporting
} Parser;

typedef enum {
    OP_NULL,
} OP;

typedef struct {
    // dynamic array of operations and opcodes
    // This relies on the fact that an enum is just a number underneath
    DA(uint8_t, code);
} Chunk;

typedef struct {
    const char* name;  // function name - should be `main` by default
    int arity;         // how many parameters
    Chunk chunk;
} AriaFunction;

typedef enum {
    INTEGER,
    BOOLEAN,
    FUNCTION,
} ValueTag;

typedef struct {
    ValueTag tag;
    union {
        int i;
        bool b;
        AriaFunction* func;
    } as;
} AriaValue;

DA(AriaValue, ValueArray);

///// Utility Functions

uint32_t hash(const char* data, size_t len) {
    uint32_t hash = 2166136261u;
    for (int i = 0; i < len; i++) {
        hash ^= (uint8_t)data[i];
        hash *= 16777619;
    }

    return hash;
}

String make_str_from_cstr(const char* data) {
    const size_t len = strlen(data);
    String str = {0};
    str.data = malloc(sizeof(char) * len);
    memcpy(str.data, data, len);
    str.len = len;

    str.hash = hash(str.data, len);
    return str;
}

String make_str_from_offset(String input, size_t offset, size_t size) {
    String str = {0};
    str.data = malloc(sizeof(char) * size);
    memcpy(str.data, input.data + offset, size);
    str.len = size;

    str.hash = hash(str.data, size);
    return str;
}

bool str_cmp(String* a, String* b) {
    if (a->len != b->len) { return false; }
    for (size_t i = 0; i < a->len; i++) {
        if (a->data[i] != b->data[i]) { return false; }
    }

    return true;
}

HashTableElem* table_find(Table* tbl, String* key) {
    uint8_t idx = key->hash % 16;
    struct buckets entry = tbl->buckets[idx];

    for (int i = 0; i < entry.count; i++) {
        if (str_cmp(&entry.items[i].key, key)) { return &entry.items[i]; }
    }

    return NULL;
}

Value* table_get(Table* tbl, String* key) {
    HashTableElem* found = table_find(tbl, key);
    if (found == NULL) { return NULL; }
    return &found->value;
}

void table_set(Table* tbl, String key, Value val) {
    HashTableElem* found = table_find(tbl, &key);
    if (found == NULL) {
        uint8_t idx = key.hash % 16;
        struct buckets entry = tbl->buckets[idx];

        HashTableElem insertable = (HashTableElem){key, val};
        da_append(&entry, insertable);
    }
}

void aria_vm_cleanup(AriaVM* vm) {
    for (int i = vm->strings->count; i >= 0; i--) {
        String str = vm->strings->items[i];
        free(str.data);
    }

    free(vm->strings->items);
}

///// Frontend

// Lexer
static size_t pc;

Token makeToken(TokType type, int start, int length) {
    // TODO: line
    Token tok = (Token){.type = type, .start = start, .length = length, .line = 0};
    return tok;
}

char peek(String input) { return input.data[pc]; }
char peekNext(String input) {
    if (pc == input.len) { return '\0'; }
    return input.data[pc++];
}

void advanceChar() { pc++; }

void skipWhitespace(String input) {
    while (isspace(peek(input))) { advanceChar(); }
}

void advanceComment(String input) {
    while (peek(input) != '\n') { advanceChar(); }
}

Token scanEqualVariant(String input, TokType single, TokType equal, int start) {
    if (peekNext(input) == '=') {
        advanceChar();
        advanceChar();
        return makeToken(equal, start, 2);
    }

    advanceChar();
    return makeToken(single, start, 1);
}

Token scanStringLiteral(String input, int start) {
    assert(peek(input) == '"');
    int length = 0;

    while (peekNext(input) != '"' && peekNext(input) != '\0') {
        advanceChar();
        length++;
    }

    if (peekNext(input) == '\0') {
        // TODO: Handle error case
        return makeToken(TOK_EOF, start, 0);
    }

    advanceChar();  // consume closing quote
    length++;
    assert(peek(input) == '"');
    advanceChar();  // Next char
    length++;

    return makeToken(TOK_STRING_LIT, start, length);
}

Token scanNumber(String input, int start) {
    // TODO: handle other numeric formats (hex, bin, oct)
    int length = 0;

    do {
        advanceChar();
        length++;
    } while (isdigit(peek(input)) || peek(input) == '.');

    return makeToken(TOK_NUM_LIT, start, length);
}

Token scanIdentifier(String input, int start) {
    int length = 0;

    do {
        length++;
        advanceChar();
    } while (isalnum(peek(input)) || peek(input) == '_');

    // Check if it's a keyword
    for (int i = 0; i < keyword_count; i++) {
        if (keywords[i].len == length && strncmp(&input.data[start], keywords[i].kw, length) == 0) {
            return makeToken(keywords[i].tok, start, length);
        }
    }

    return makeToken(TOK_IDENTIFIER, start, length);
}

Token scanToken(String input) {
    skipWhitespace(input);

    int start = pc;
    char c = peek(input);

    if (c == '\0') { return makeToken(TOK_EOF, start, 0); }
    switch (c) {
            // clang-format off
        case ',': advanceChar(); return makeToken(TOK_COMMA, start, 1);
        case ';': advanceComment(input); return makeToken(TOK_COUNT, 0, 0);
        case ':': advanceChar(); return makeToken(TOK_COLON, start, 1);
        case '-': advanceChar(); return makeToken(TOK_MINUS, start, 1);
        case '+': advanceChar(); return makeToken(TOK_PLUS, start, 1);
        case '*': advanceChar(); return makeToken(TOK_STAR, start, 1);
        case '/': advanceChar(); return makeToken(TOK_SLASH, start, 1);
        case '[': advanceChar(); return makeToken(TOK_LEFT_SQUACKET, start, 1);
        case ']': advanceChar(); return makeToken(TOK_RIGHT_SQUACKET, start, 1);
        case '(': advanceChar(); return makeToken(TOK_LEFT_PAREN, start, 1);
        case ')': advanceChar(); return makeToken(TOK_RIGHT_PAREN, start, 1);
        case '!': return scanEqualVariant(input, TOK_BANG, TOK_BANG_EQUAL, start);
        case '=': return scanEqualVariant(input, TOK_EQUAL, TOK_EQUAL_EQUAL, start);
        case '<': return scanEqualVariant(input, TOK_LESS, TOK_LESS_EQUAL, start);
        case '>': return scanEqualVariant(input, TOK_GREATER, TOK_GREATER_EQUAL, start);
        case '"': return scanStringLiteral(input, start);
        case '&':
                  if (peek(input) == '&') {
                      advanceChar();
                      advanceChar();
                      return makeToken(TOK_AND, start, 2);
                  }
                  break;
        case '|':
                  if (peek(input) == '|') {
                      advanceChar();
                      advanceChar();
                      return makeToken(TOK_OR, start, 2);
                  }
                  break;
        case '.':
                  advanceChar();
                  if (peek(input) == '.' && peekNext(input) == '.') {
                      advanceChar();
                      advanceChar();
                      return makeToken(TOK_ELLIPSIS, start, 3);
                  }
                  return makeToken(TOK_DOT, start, 1);
                  break;
    }

    //clang-format on

    if (isdigit(c)) { return scanNumber(input, start); }
    if (isalpha(c) || c == '_') { return scanIdentifier(input, start); }

    // TODO: Handle error case
    return makeToken(TOK_ERROR, start, 0);
}

// Parser

bool check(Parser* parser, TokType type) {
    return parser->current.type == type;
}

bool match(Parser* parser, TokType type) {
    if (!check(parser, type)) { return false; }

    parser->prev = parser->current;
    parser->current = scanToken(parser->input);
    return true;
}

void declaration(Parser* parser) {
    if (match(parser, TOK_VAR)) {
        varDeclaration();
    }
}

void parse(const char* buf) {
    // While the current token isn't EOF, we pass the current token
    // into the top of the RD parser. The RD emits bytes straight
    // into the chunk to construct the AriaFunction obj

    pc = 0;
    Parser parser = {0};
    parser.input = make_str_from_cstr(buf);
    parser.current = scanToken(parser.input);

    // TODO: imports and functions here?

    while (parser.current.type != TOK_EOF) {
        printf("Token: %d start(%d), len(%d)\n", parser.current.type, parser.current.start, parser.current.length);
        declaration(&parser);
    }
}

///// Backend

void execute(AriaFunction* func) {
    // take the chunk from the function, iterate through the ops and execute as relevant
    // This is another dispatching type method, each operation will likely need it's own
    // C function for executing
}

///// Runtime
Status aria_load_file(AriaVM* vm, const char* filepath) {
    // Read file into chars
    // call frontend then backend

    FILE* fp = fopen(filepath, "r");
    if (fp == NULL) { return STATUS_FILENOTFOUND; }

    fseek(fp, 0, SEEK_END);
    size_t length = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* buf = malloc(length);
    fread(buf, 1, length, fp);

    parse(buf);
}

Status aria_call_func(AriaVM* vm, const char* func) {
    return STATUS_OK;
}
