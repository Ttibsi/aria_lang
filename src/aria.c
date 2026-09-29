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

///// Macros
#define ARIA_DEBUG

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

#define checkError(func)                \
    do {                                \
        if (func) {                     \
            P->hasError = true;         \
            syncToStatementBoundary(P); \
            return;                     \
        }                               \
    } while (0)

/// Structs and Enums
typedef struct {
    char* data;
    size_t len;

    uint32_t hash;
} String;

DA(String, Strings);

typedef struct AriaValue AriaValue;
typedef struct {
    String key;
    AriaValue* value;
} HashTableElem;

typedef struct Table {
    DA(HashTableElem, buckets)[16];
} Table;

typedef enum {
    // Single-character tokens.
    TOK_LEFT_PAREN,      // 0
    TOK_RIGHT_PAREN,     // 1
    TOK_LEFT_SQUACKET,   // 2
    TOK_RIGHT_SQUACKET,  // 3
    TOK_COMMA,           // 4
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
    TOK_AND,  // 20
    TOK_OR,   // 21

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
    TOK_VAR,      // 45

    // Literals.
    TOK_IDENTIFIER,  // 46
    TOK_STRING_LIT,  // 47
    TOK_CHAR_LIT,    // 48
    TOK_NUM_LIT,     // 49

    TOK_EOF,  // 51
    TOK_ERROR,
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
    {"VAR",     3, TOK_VAR     }, // 45
};
static const int keyword_count = sizeof(keywords) / sizeof(keywords[0]);
// clang-format on

typedef struct {
    Token prev;
    Token current;
    String input;
    bool hasError;
} Parser;

typedef enum {
    OP_NULL,
    OP_SET_VAR,
    OP_GET_VAR,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_CONST_NUM,
    OP_CONST_STR,
    OP_CONST_CHAR,
    OP_DEFINE_GLOBAL,
    OP_JUMP_IF_FALSE,
    OP_JUMP,
    OP_POP,
    OP_LOOP,
    OP_RETURN,
    OP_CALL,
    OP_NOT,
    OP_NEGATE,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_EQUAL,
    OP_NOT_EQUAL,
    OP_GREATER,
    OP_GREATER_EQUAL,
    OP_LESS,
    OP_LESS_EQUAL,
    OP_AND,
    OP_OR,
} OP;

typedef struct {
    // dynamic array of operations and opcodes
    // This relies on the fact that an enum is just a number underneath
    DA(uint8_t, code);
} Chunk;

typedef struct {
    String name;  // function name - should be `main` by default
    String ret_type;
    int arity;  // how many parameters
    String globals[UINT8_MAX + 1];
    uint8_t global_count;
    Chunk chunk;
} AriaFunction;

typedef enum {
    INTEGER,
    BOOLEAN,
    FUNCTION,
} ValueTag;

struct AriaValue {
    ValueTag tag;
    union {
        int i;
        bool b;
        AriaFunction* func;
    } as;
};

DA(AriaValue, ValueArray);

typedef enum {
    PREC_NONE = 0,
    PREC_OR,
    PREC_AND,
    PREC_EQUALITY,
    PREC_COMPARISON,
    PREC_TERM,
    PREC_FACTOR,
    PREC_UNARY,
} Precedence;

///// Utility Functions

uint32_t hash(const char* data, size_t len) {
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < len; i++) {
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
    if (tbl == NULL) { return NULL; }
    uint8_t idx = key->hash % 16;
    struct buckets* entry = &tbl->buckets[idx];

    for (int i = 0; i < entry->count; i++) {
        if (str_cmp(&entry->items[i].key, key)) { return &entry->items[i]; }
    }

    return NULL;
}

AriaValue* table_get(Table* tbl, String* key) {
    HashTableElem* found = table_find(tbl, key);
    if (found == NULL) { return NULL; }
    return found->value;
}

void table_set(Table* tbl, String key, AriaValue* val) {
    if (tbl == NULL) { return; }
    HashTableElem* found = table_find(tbl, &key);
    if (found != NULL) {
        *(found->value) = *val;
        return;
    }

    uint8_t idx = key.hash % 16;
    struct buckets* entry = &tbl->buckets[idx];
    AriaValue* stored = malloc(sizeof(AriaValue));
    *stored = *val;

    HashTableElem insertable = (HashTableElem){key, stored};
    da_append(entry, insertable);
}

void aria_vm_cleanup(AriaVM* vm) {
    if (vm->functions != NULL) {
        for (int i = 0; i < 16; i++) {
            struct buckets* bucket = &vm->functions->buckets[i];
            for (int j = 0; j < bucket->count; j++) { free(bucket->items[j].value); }
            free(bucket->items);
        }
        free(vm->functions);
    }

    if (vm->strings != NULL) {
        for (int i = vm->strings->count - 1; i >= 0; i--) {
            String str = vm->strings->items[i];
            free(str.data);
        }
        free(vm->strings->items);
        free(vm->strings);
    }

    if (vm->moduleImports != NULL) {
        for (int i = 0; i < vm->moduleImports->count; i++) {
            String str = vm->moduleImports->items[i];
            free(str.data);
        }
        free(vm->moduleImports->items);
        free(vm->moduleImports);
    }

    if (vm->fileImports != NULL) {
        for (int i = 0; i < vm->fileImports->count; i++) {
            String str = vm->fileImports->items[i];
            free(str.data);
        }
        free(vm->fileImports->items);
        free(vm->fileImports);
    }
}

///// Frontend

// Lexer
static size_t pc;

Token makeToken(TokType type, int start, int length) {
#ifdef ARIA_DEBUG
    printf("Token: %d, start: %d, length: %d\n", type, start, length);
#endif

    // TODO: line
    Token tok = (Token){.type = type, .start = start, .length = length, .line = 0};
    return tok;
}

char peek(String input) { return input.data[pc]; }
char peekNext(String input) {
    if (pc + 1 >= input.len) { return '\0'; }
    return input.data[pc + 1];
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
    if (c == ';') {
        advanceComment(input);
        c = peek(input);
    }

    switch (c) {
            // clang-format off
        case ',': advanceChar(); return makeToken(TOK_COMMA, start, 1);
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
    }

    //clang-format on

    if (isdigit(c)) { return scanNumber(input, start); }
    if (isalpha(c) || c == '_') { return scanIdentifier(input, start); }

    // TODO: Handle error case
    advanceChar();
    return makeToken(TOK_ERROR, start, 1);
}

// Parser
void parseBlock(Parser* P, AriaFunction* func, bool* emittedReturn);
bool parsePrecedence(Parser* P, AriaFunction* func, Precedence minPrec, const TokType* stops, int stopCount);

bool check(Parser* parser, TokType type) {
    return parser->current.type == type;
}

bool checkType(Parser* parser) {
    TokType curType = parser->current.type;
    if (curType == TOK_BOOL || curType == TOK_CHAR || curType == TOK_NIL || curType == TOK_NUM || curType == TOK_STR ||
        curType == TOK_LIST || curType == TOK_MAP) {
        return true;
    }

    return false;
}

bool match(Parser* parser, TokType type) {
    if (!check(parser, type)) { return false; }

    parser->prev = parser->current;
    parser->current = scanToken(parser->input);
    return true;
}

void advanceTok(Parser* P) {
    P->prev = P->current;
    P->current = scanToken(P->input);
}

// TODO: add a Status field to the parser, set the Status with new error types properly
// Do we want a "printReadableError" function too that prints more information based on enum value?
// Then in parse() we return a Status and can handle that properly in aria_load_file()
void setError(Parser* P) {
    P->hasError = true;
    advanceTok(P);
}

void emitByte(AriaFunction* func, uint8_t byte) { da_append(&func->chunk.code, byte); }

void emitBytes(AriaFunction* func, uint8_t byte1, uint8_t byte2) {
    emitByte(func, byte1);
    emitByte(func, byte2);
}

int emitJump(AriaFunction* func, uint8_t instruction) {
    emitByte(func, instruction);
    emitByte(func, 0xff);
    emitByte(func, 0xff);
    return func->chunk.code.count - 2;
}

void patchJump(AriaFunction* func, int offset) {
    int jump = func->chunk.code.count - offset - 2;
    if (jump > UINT16_MAX) { return; }

    func->chunk.code.items[offset] = (jump >> 8) & 0xff;
    func->chunk.code.items[offset + 1] = jump & 0xff;
}

void emitLoop(AriaFunction* func, int loopStart) {
    emitByte(func, OP_LOOP);
    int offset = func->chunk.code.count - loopStart + 2;
    if (offset > UINT16_MAX) { return; }

    emitByte(func, (offset >> 8) & 0xff);
    emitByte(func, offset & 0xff);
}

uint8_t identifierConstant(Parser* P, AriaFunction* func, Token nameTok) {
    if (func->global_count == UINT8_MAX) {
        P->hasError = true;
        return 0;
    }

    uint8_t idx = func->global_count;
    func->globals[idx] = make_str_from_offset(P->input, nameTok.start, nameTok.length);
    func->global_count++;
    return idx;
}

void parseImport(Parser* P, AriaVM* vm) {
    assert(P->current.type == TOK_IMPORT);
    match(P, TOK_IMPORT);

    if (check(P, TOK_EOF)) {
        P->hasError = true;
        return;
    }

    if (!(check(P, TOK_IDENTIFIER) || check(P, TOK_STRING_LIT))) {
        setError(P);
        return;
    }

    String importName = make_str_from_offset(P->input, P->current.start, P->current.length);
    if (check(P, TOK_STRING_LIT)) {
        if (vm->fileImports == NULL) { vm->fileImports = calloc(1, sizeof(Strings)); }
        da_append(vm->fileImports, importName);
    } else {
        if (vm->moduleImports == NULL) { vm->moduleImports = calloc(1, sizeof(Strings)); }
        da_append(vm->moduleImports, importName);
    }

    match(P, P->current.type);
}

bool isStatementBoundary(Parser* P) {
    switch (P->current.type) {
        case TOK_VAR:
        case TOK_IF:
        case TOK_FOR:
        case TOK_FOREACH:
        case TOK_RET:
        case TOK_END:
        case TOK_ELSE:
        case TOK_EOF:
            return true;
        default:
            return false;
    }
}

// Used for error recovery
void syncToStatementBoundary(Parser* P) {
    while (!isStatementBoundary(P)) { advanceTok(P); }
}


bool isExprStop(Parser* P, const TokType* stops, int stopCount) {
    for (int i = 0; i < stopCount; i++) {
        if (check(P, stops[i])) { return true; }
    }
    return false;
}

Precedence getInfixPrecedence(TokType type) {
    switch (type) {
        case TOK_OR:
            return PREC_OR;
        case TOK_AND:
            return PREC_AND;
        case TOK_EQUAL_EQUAL:
        case TOK_BANG_EQUAL:
            return PREC_EQUALITY;
        case TOK_GREATER:
        case TOK_GREATER_EQUAL:
        case TOK_LESS:
        case TOK_LESS_EQUAL:
            return PREC_COMPARISON;
        case TOK_PLUS:
        case TOK_MINUS:
            return PREC_TERM;
        case TOK_STAR:
        case TOK_SLASH:
            return PREC_FACTOR;
        default:
            return PREC_NONE;
    }
}

void emitBinaryOp(AriaFunction* func, TokType op) {
    switch (op) {
        case TOK_OR:
            emitByte(func, OP_OR);
            break;
        case TOK_AND:
            emitByte(func, OP_AND);
            break;
        case TOK_EQUAL_EQUAL:
            emitByte(func, OP_EQUAL);
            break;
        case TOK_BANG_EQUAL:
            emitByte(func, OP_NOT_EQUAL);
            break;
        case TOK_GREATER:
            emitByte(func, OP_GREATER);
            break;
        case TOK_GREATER_EQUAL:
            emitByte(func, OP_GREATER_EQUAL);
            break;
        case TOK_LESS:
            emitByte(func, OP_LESS);
            break;
        case TOK_LESS_EQUAL:
            emitByte(func, OP_LESS_EQUAL);
            break;
        case TOK_PLUS:
            emitByte(func, OP_ADD);
            break;
        case TOK_MINUS:
            emitByte(func, OP_SUBTRACT);
            break;
        case TOK_STAR:
            emitByte(func, OP_MULTIPLY);
            break;
        case TOK_SLASH:
            emitByte(func, OP_DIVIDE);
            break;
        default:
            break;
    }
}


bool parsePrefix(Parser* P, AriaFunction* func, const TokType* stops, int stopCount) {
    TokType tok = P->current.type;
    switch (tok) {
        case TOK_NUM_LIT:
            emitByte(func, OP_CONST_NUM);
            match(P, TOK_NUM_LIT);
            return true;
        case TOK_STRING_LIT:
            emitByte(func, OP_CONST_STR);
            match(P, TOK_STRING_LIT);
            return true;
        case TOK_CHAR_LIT:
            emitByte(func, OP_CONST_CHAR);
            match(P, TOK_CHAR_LIT);
            return true;
        case TOK_TRUE:
            emitByte(func, OP_TRUE);
            match(P, TOK_TRUE);
            return true;
        case TOK_FALSE:
            emitByte(func, OP_FALSE);
            match(P, TOK_FALSE);
            return true;
        case TOK_NIL:
            emitByte(func, OP_NIL);
            match(P, TOK_NIL);
            return true;
        case TOK_IDENTIFIER: {
            Token nameTok = P->current;
            match(P, TOK_IDENTIFIER);
            uint8_t nameIdx = identifierConstant(P, func, nameTok);
            emitBytes(func, OP_GET_VAR, nameIdx);
            return true;
        }
        case TOK_LEFT_PAREN: {
            match(P, TOK_LEFT_PAREN);
            TokType groupStop[1] = {TOK_RIGHT_PAREN};
            if (!parsePrecedence(P, func, PREC_OR, groupStop, 1)) { return false; }
            if (!match(P, TOK_RIGHT_PAREN)) {
                P->hasError = true;
                return false;
            }
            return true;
        }
        case TOK_BANG:
            match(P, TOK_BANG);
            if (!parsePrecedence(P, func, PREC_UNARY, stops, stopCount)) { return false; }
            emitByte(func, OP_NOT);
            return true;
        case TOK_MINUS:
            match(P, TOK_MINUS);
            if (!parsePrecedence(P, func, PREC_UNARY, stops, stopCount)) { return false; }
            emitByte(func, OP_NEGATE);
            return true;
        default:
            P->hasError = true;
            return false;
    }
}

bool parsePrecedence(Parser* P, AriaFunction* func, Precedence minPrec, const TokType* stops, int stopCount) {
    if (check(P, TOK_EOF) || isExprStop(P, stops, stopCount)) {
        P->hasError = true;
        return false;
    }

    if (!parsePrefix(P, func, stops, stopCount)) { return false; }

    while (!(check(P, TOK_EOF) || isExprStop(P, stops, stopCount))) {
        Precedence prec = getInfixPrecedence(P->current.type);
        if (prec < minPrec || prec == PREC_NONE) { break; }

        TokType op = P->current.type;
        match(P, op);
        if (!parsePrecedence(P, func, (Precedence)(prec + 1), stops, stopCount)) { return false; }
        emitBinaryOp(func, op);
    }

    return true;
}

bool parseExpressionUntil(Parser* P, AriaFunction* func, const TokType* stops, int stopCount) {
    return parsePrecedence(P, func, PREC_OR, stops, stopCount);
}

void parseVariable(Parser* P, AriaFunction* func) {
    assert(P->current.type == TOK_VAR);
    match(P, TOK_VAR);

    checkError(!check(P, TOK_IDENTIFIER));
    Token nameTok = P->current;
    match(P, TOK_IDENTIFIER);

    checkError(!checkType(P));
    Token declaredType = P->current;
    match(P, declaredType.type);

    if (declaredType.type == TOK_LIST || declaredType.type == TOK_MAP) {
        checkError(!match(P, TOK_LEFT_SQUACKET));
        checkError(!checkType(P));
        match(P, P->current.type);

        if (declaredType.type == TOK_MAP) {
            checkError(!match(P, TOK_COMMA));
            checkError(!checkType(P));
            match(P, P->current.type);
        }

        checkError(!match(P, TOK_RIGHT_SQUACKET));
    }

    if (match(P, TOK_EQUAL)) {
        TokType initStops[7] = {TOK_VAR, TOK_IF, TOK_FOR, TOK_FOREACH, TOK_RET, TOK_END, TOK_ELSE};
        checkError(!parseExpressionUntil(P, func, initStops, 7));
    } else {
        emitByte(func, OP_NIL);
    }

    uint8_t nameIdx = identifierConstant(P, func, nameTok);
    emitBytes(func, OP_DEFINE_GLOBAL, nameIdx);
    emitByte(func, (uint8_t)declaredType.type);
}

void parseIf(Parser* P, AriaFunction* func, bool* emittedReturn) {
    assert(P->current.type == TOK_IF);
    match(P, TOK_IF);

    if (check(P, TOK_EOF)) {
        P->hasError = true;
        return;
    }

    TokType condStops[1] = {TOK_THEN};
    if (!parseExpressionUntil(P, func, condStops, 1)) {
        P->hasError = true;
        return;
    }
    if (!match(P, TOK_THEN)) { P->hasError = true; }

    int thenJump = emitJump(func, OP_JUMP_IF_FALSE);
    emitByte(func, OP_POP);

    parseBlock(P, func, emittedReturn);

    int elseJump = emitJump(func, OP_JUMP);
    patchJump(func, thenJump);
    emitByte(func, OP_POP);

    if (match(P, TOK_ELSE)) {
        if (check(P, TOK_IF)) {
            parseIf(P, func, emittedReturn);
            patchJump(func, elseJump);
            return;
        }
        parseBlock(P, func, emittedReturn);
    }

    patchJump(func, elseJump);
    if (!match(P, TOK_END)) { P->hasError = true; }
}

void parseFor(Parser* P, AriaFunction* func, bool* emittedReturn) {
    assert(P->current.type == TOK_FOR);
    match(P, TOK_FOR);

    if (check(P, TOK_EOF)) {
        P->hasError = true;
        return;
    }

    checkError(!check(P, TOK_IDENTIFIER));
    match(P, TOK_IDENTIFIER);

    checkError(!match(P, TOK_EQUAL));
    TokType startStops[1] = {TOK_TO};
    if (!parseExpressionUntil(P, func, startStops, 1)) {
        P->hasError = true;
        return;
    }

    checkError(!match(P, TOK_TO));

    TokType stopStops[2] = {TOK_STEP, TOK_THEN};
    if (!parseExpressionUntil(P, func, stopStops, 2)) {
        P->hasError = true;
        return;
    }

    if (match(P, TOK_STEP)) {
        TokType stepStops[1] = {TOK_THEN};
        if (!parseExpressionUntil(P, func, stepStops, 1)) {
            P->hasError = true;
            return;
        }
    }

    checkError(!match(P, TOK_THEN));
    int loopStart = func->chunk.code.count;
    int exitJump = emitJump(func, OP_JUMP_IF_FALSE);
    emitByte(func, OP_POP);

    parseBlock(P, func, emittedReturn);

    emitLoop(func, loopStart);
    patchJump(func, exitJump);
    emitByte(func, OP_POP);

    if (!match(P, TOK_END)) { P->hasError = true; }
}

void parseForEach(Parser* P, AriaFunction* func, bool* emittedReturn) {
    assert(P->current.type == TOK_FOREACH);
    match(P, TOK_FOREACH);

    checkError(!check(P, TOK_IDENTIFIER));
    match(P, TOK_IDENTIFIER);

    checkError(!match(P, TOK_COMMA));
    checkError(!check(P, TOK_IDENTIFIER));
    match(P, TOK_IDENTIFIER);

    checkError(!match(P, TOK_IN));

    checkError(!check(P, TOK_IDENTIFIER));
    match(P, TOK_IDENTIFIER);

    checkError(!match(P, TOK_THEN));

    int loopStart = func->chunk.code.count;
    int exitJump = emitJump(func, OP_JUMP_IF_FALSE);
    emitByte(func, OP_POP);

    parseBlock(P, func, emittedReturn);

    emitLoop(func, loopStart);
    patchJump(func, exitJump);
    emitByte(func, OP_POP);

    if (!match(P, TOK_END)) { P->hasError = true; }
}

void parseReturn(Parser* P, AriaFunction* func, bool* emittedReturn) {
    assert(P->current.type == TOK_RET);
    match(P, TOK_RET);

    if (check(P, TOK_EOF)) {
        P->hasError = true;
        return;
    }

    // Return is terminated by the surrounding block boundary.
    if (check(P, TOK_END) || check(P, TOK_ELSE)) {
        emitByte(func, OP_NIL);
        emitByte(func, OP_RETURN);
        *emittedReturn = true;
        return;
    }

    TokType retStops[2] = {TOK_END, TOK_ELSE};
    if (!parseExpressionUntil(P, func, retStops, 2)) {
        P->hasError = true;
        return;
    }

    emitByte(func, OP_RETURN);
    *emittedReturn = true;
}

void parseIdentifier(Parser* P, AriaFunction* func) {
    assert(P->current.type == TOK_IDENTIFIER);
    int stmtStart = func->chunk.code.count;
    Token nameTok = P->current;
    match(P, TOK_IDENTIFIER);

    if (match(P, TOK_LEFT_PAREN)) {
        int args_idx = 0;
        while (!check(P, TOK_RIGHT_PAREN)) {
            if (check(P, TOK_EOF)) {
                P->hasError = true;
                func->chunk.code.count = stmtStart;
                return;
            }

            TokType argStops[2] = {TOK_COMMA, TOK_RIGHT_PAREN};
            if (!parseExpressionUntil(P, func, argStops, 2)) {
                P->hasError = true;
                func->chunk.code.count = stmtStart;
                return;
            }
            args_idx++;

            if (match(P, TOK_COMMA)) {
                if (check(P, TOK_RIGHT_PAREN)) {
                    P->hasError = true;
                    func->chunk.code.count = stmtStart;
                    return;
                }
                continue;
            }

            if (!check(P, TOK_RIGHT_PAREN)) {
                P->hasError = true;
                func->chunk.code.count = stmtStart;
                syncToStatementBoundary(P);
                return;
            }
        }
        if (!match(P, TOK_RIGHT_PAREN)) {
            P->hasError = true;
            func->chunk.code.count = stmtStart;
            return;
        }

        uint8_t nameIdx = identifierConstant(P, func, nameTok);
        emitByte(func, OP_CALL);
        emitByte(func, nameIdx);
        emitByte(func, (uint8_t)args_idx);
        return;
    }

    if (match(P, TOK_EQUAL)) {
        if (check(P, TOK_EOF) || check(P, TOK_END) || check(P, TOK_ELSE)) {
            P->hasError = true;
            func->chunk.code.count = stmtStart;
            return;
        }

        TokType rhsStops[2] = {TOK_END, TOK_ELSE};
        if (!parseExpressionUntil(P, func, rhsStops, 2)) {
            P->hasError = true;
            func->chunk.code.count = stmtStart;
            return;
        }

        uint8_t nameIdx = identifierConstant(P, func, nameTok);
        emitByte(func, OP_SET_VAR);
        emitByte(func, nameIdx);
        return;
    }

    P->hasError = true;
    func->chunk.code.count = stmtStart;
    syncToStatementBoundary(P);
}

void parseStatement(Parser* P, AriaFunction* func, bool* emittedReturn) {
    if (check(P, TOK_EOF)) {
        P->hasError = true;
        return;
    }

    switch (P->current.type) {
        case TOK_VAR:
            parseVariable(P, func);
            break;
        case TOK_IF:
            parseIf(P, func, emittedReturn);
            break;
        case TOK_FOR:
            parseFor(P, func, emittedReturn);
            break;
        case TOK_FOREACH:
            parseForEach(P, func, emittedReturn);
            break;
        case TOK_RET:
            parseReturn(P, func, emittedReturn);
            break;
        case TOK_IDENTIFIER:
            parseIdentifier(P, func);
            break;
        default:
            setError(P);
            break;
    }
}

void parseBlock(Parser* P, AriaFunction* func, bool* emittedReturn) {
    while (!(check(P, TOK_END) || check(P, TOK_ELSE) || check(P, TOK_EOF))) {
        TokType start = P->current.type;
        parseStatement(P, func, emittedReturn);
        if (P->current.type == start) { setError(P); }
    }
}

AriaValue parseFunc(Parser* P) {
    AriaFunction* func = malloc(sizeof(AriaFunction));
    *func = (AriaFunction){0};
    bool emittedReturn = false;

    // Function header
    if (!match(P, TOK_FUNC)) { P->hasError = true; }
    if (!check(P, TOK_IDENTIFIER)) {
        P->hasError = true;
        setError(P);
    } else {
        Token tok = P->current;
        func->name = make_str_from_offset(P->input, tok.start, tok.length);
        match(P, TOK_IDENTIFIER);
    }

    // Function args
    if (!match(P, TOK_LEFT_PAREN)) { P->hasError = true; }
    int args_idx = 0;
    while (!check(P, TOK_RIGHT_PAREN)) {
        if (!check(P, TOK_IDENTIFIER)) {
            setError(P);
            continue;
        }
        match(P, TOK_IDENTIFIER);

        TokType argType = P->current.type;
        if (!checkType(P)) {
            P->hasError = true;
            if (check(P, TOK_RIGHT_PAREN)) { break; }
            advanceTok(P);
            continue;
        }
        match(P, argType);

        if (argType == TOK_LIST || argType == TOK_MAP) {
            if (!match(P, TOK_LEFT_SQUACKET)) { P->hasError = true; }

            if (!checkType(P)) {
                P->hasError = true;
                if (!(check(P, TOK_COMMA) || check(P, TOK_RIGHT_SQUACKET) || check(P, TOK_RIGHT_PAREN))) {
                    advanceTok(P);
                }
            } else {
                match(P, P->current.type);
            }

            if (argType == TOK_MAP) {
                if (!match(P, TOK_COMMA)) { P->hasError = true; }
                if (!checkType(P)) {
                    P->hasError = true;
                    if (!(check(P, TOK_RIGHT_SQUACKET) || check(P, TOK_RIGHT_PAREN))) {
                        advanceTok(P);
                    }
                } else {
                    match(P, P->current.type);
                }
            }

            if (!match(P, TOK_RIGHT_SQUACKET)) { P->hasError = true; }
        }

        args_idx++;
        if (check(P, TOK_COMMA)) { match(P, TOK_COMMA); }
    }
    if (!match(P, TOK_RIGHT_PAREN)) { P->hasError = true; }
    func->arity = args_idx;

    // Function return type
    if (!checkType(P)) {
        setError(P);
    } else {
        Token retTok = P->current;
        func->ret_type = make_str_from_offset(P->input, retTok.start, retTok.length);
        match(P, retTok.type);
    }

    // Function body
    parseBlock(P, func, &emittedReturn);
    if (!match(P, TOK_END)) { P->hasError = true; }
    if (!emittedReturn) {
        emitByte(func, OP_NIL);
        emitByte(func, OP_RETURN);
    }

    AriaValue funcValue = {0};
    funcValue.tag = FUNCTION;
    funcValue.as.func = func;
    return funcValue;
}

const char* op_name(uint8_t op) {
    switch ((OP)op) {
        case OP_NULL: return "OP_NULL";
        case OP_SET_VAR: return "OP_SET_VAR";
        case OP_GET_VAR: return "OP_GET_VAR";
        case OP_NIL: return "OP_NIL";
        case OP_TRUE: return "OP_TRUE";
        case OP_FALSE: return "OP_FALSE";
        case OP_CONST_NUM: return "OP_CONST_NUM";
        case OP_CONST_STR: return "OP_CONST_STR";
        case OP_CONST_CHAR: return "OP_CONST_CHAR";
        case OP_DEFINE_GLOBAL: return "OP_DEFINE_GLOBAL";
        case OP_JUMP_IF_FALSE: return "OP_JUMP_IF_FALSE";
        case OP_JUMP: return "OP_JUMP";
        case OP_POP: return "OP_POP";
        case OP_LOOP: return "OP_LOOP";
        case OP_RETURN: return "OP_RETURN";
        case OP_CALL: return "OP_CALL";
        case OP_NOT: return "OP_NOT";
        case OP_NEGATE: return "OP_NEGATE";
        case OP_ADD: return "OP_ADD";
        case OP_SUBTRACT: return "OP_SUBTRACT";
        case OP_MULTIPLY: return "OP_MULTIPLY";
        case OP_DIVIDE: return "OP_DIVIDE";
        case OP_EQUAL: return "OP_EQUAL";
        case OP_NOT_EQUAL: return "OP_NOT_EQUAL";
        case OP_GREATER: return "OP_GREATER";
        case OP_GREATER_EQUAL: return "OP_GREATER_EQUAL";
        case OP_LESS: return "OP_LESS";
        case OP_LESS_EQUAL: return "OP_LESS_EQUAL";
        case OP_AND: return "OP_AND";
        case OP_OR: return "OP_OR";
        default: return "OP_UNKNOWN";
    }
}

void dump_function_ops(AriaFunction* func) {
    printf("=== Function %.*s (%d bytes) ===\n", (int)func->name.len, func->name.data, func->chunk.code.count);
    for (int i = 0; i < func->chunk.code.count; i++) {
        uint8_t op = func->chunk.code.items[i];
        printf("%04d  %s", i, op_name(op));

        switch ((OP)op) {
            case OP_SET_VAR:
            case OP_GET_VAR:
                if (i + 1 < func->chunk.code.count) {
                    printf(" %u", func->chunk.code.items[i + 1]);
                    i += 1;
                }
                break;
            case OP_DEFINE_GLOBAL:
                if (i + 2 < func->chunk.code.count) {
                    printf(" name=%u type=%u", func->chunk.code.items[i + 1], func->chunk.code.items[i + 2]);
                    i += 2;
                }
                break;
            case OP_JUMP_IF_FALSE:
            case OP_JUMP:
            case OP_LOOP:
                if (i + 2 < func->chunk.code.count) {
                    uint16_t offset = (uint16_t)((func->chunk.code.items[i + 1] << 8) | func->chunk.code.items[i + 2]);
                    printf(" %u", offset);
                    i += 2;
                }
                break;
            case OP_CALL:
                if (i + 2 < func->chunk.code.count) {
                    printf(" callee=%u argc=%u", func->chunk.code.items[i + 1], func->chunk.code.items[i + 2]);
                    i += 2;
                }
                break;
            default:
                break;
        }
        printf("\n");
    }
}

void dump_all_functions(AriaVM* vm) {
    for (int i = 0; i < 16; i++) {
        struct buckets* bucket = &vm->functions->buckets[i];
        for (int j = 0; j < bucket->count; j++) {
            AriaValue* value = bucket->items[j].value;
            if (value != NULL && value->tag == FUNCTION && value->as.func != NULL) {
                dump_function_ops(value->as.func);
            }
        }
    }
}

void parse(const char* buf, AriaVM* vm) {
    // While the current token isn't EOF, we pass the current token
    // into the top of the RD parser. The RD emits bytes straight
    // into the chunk to construct the AriaFunction obj

    pc = 0;
    Parser parser = {0};
    parser.input = make_str_from_cstr(buf);
    parser.current = scanToken(parser.input);
    if (vm->functions == NULL) { vm->functions = calloc(1, sizeof(Table)); }
    if (vm->strings == NULL) { vm->strings = calloc(1, sizeof(Strings)); }

    while (parser.current.type != TOK_EOF) {
        if (parser.current.type == TOK_IMPORT) {
            parseImport(&parser, vm);
        } else if (parser.current.type == TOK_FUNC) {
            AriaValue funcValue = parseFunc(&parser);
            assert(funcValue.tag == FUNCTION);
            table_set(vm->functions, funcValue.as.func->name, &funcValue);
        } else {
            setError(&parser);
        }
    }
}

///// Backend

Status execute(AriaFunction* func) {
    // take the chunk from the function, iterate through the ops and execute as relevant
    // This is another dispatching type method, each operation will likely need it's own
    // C function for executing
    if (func == NULL) { return STATUS_NOMAIN; }

    struct code body = func->chunk.code;
    for (int i = 0; i < body.count; i++) {
        uint8_t inst = body.items[i];

        switch (inst) {
            case OP_NULL: break;
            case OP_SET_VAR: break;
            case OP_GET_VAR: break;
            case OP_NIL: break;
            case OP_TRUE: break;
            case OP_FALSE: break;
            case OP_CONST_NUM: break;
            case OP_CONST_STR: break;
            case OP_CONST_CHAR: break;
            case OP_DEFINE_GLOBAL: break;
            case OP_JUMP_IF_FALSE: break;
            case OP_JUMP: break;
            case OP_POP: break;
            case OP_LOOP: break;
            case OP_RETURN: break;
            case OP_CALL: break;
            case OP_NOT: break;
            case OP_NEGATE: break;
            case OP_ADD: break;
            case OP_SUBTRACT: break;
            case OP_MULTIPLY: break;
            case OP_DIVIDE: break;
            case OP_EQUAL: break;
            case OP_NOT_EQUAL: break;
            case OP_GREATER: break;
            case OP_GREATER_EQUAL: break;
            case OP_LESS: break;
            case OP_LESS_EQUAL: break;
            case OP_AND: break;
            case OP_OR: break;
        }
    }

    return STATUS_OK;
}

///// Runtime
Status aria_load_file(AriaVM* vm, const char* filepath) {
    FILE* fp = fopen(filepath, "r");
    if (fp == NULL) { return STATUS_FILENOTFOUND; }

    fseek(fp, 0, SEEK_END);
    size_t length = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* buf = malloc(length + 1);
    fread(buf, 1, length, fp);
    buf[length] = '\0';
    fclose(fp);

    parse(buf, vm);
#ifdef ARIA_DEBUG
    dump_all_functions(vm);
#endif

    Status err = STATUS_OK;
    String main = make_str_from_cstr("main");
    AriaValue* main_func = table_get(vm->functions, &main);

    err = execute(main_func->as.func);
    if (err) { goto cleanup; }


cleanup:
    free(buf);
    return STATUS_OK;
}

Status aria_call_func(AriaVM* vm, const char* func) {
    return STATUS_OK;
}
