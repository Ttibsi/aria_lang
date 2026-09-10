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
    OP_CONSTANT,
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_GET_NAME,
    OP_CALL,
    OP_LESS,
    OP_EQUAL,
    OP_ADD,
    OP_POP,
    OP_JUMP_IF_FALSE,
    OP_JUMP,
    OP_LOOP,
    OP_ITER_INIT,
    OP_ITER_NEXT,
    OP_RETURN,
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
    str.data = malloc(sizeof(char) * (len + 1));
    memcpy(str.data, data, len);
    str.data[len] = '\0';
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
    if (vm == NULL || vm->strings == NULL) { return; }

    for (int i = vm->strings->count - 1; i >= 0; i--) {
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
    if (pc + 1 >= input.len) { return '\0'; }
    return input.data[pc + 1];
}

void advanceChar() { pc++; }

void advanceComment(String input);

void skipWhitespace(String input) {
    while (true) {
        while (isspace(peek(input))) { advanceChar(); }
        if (peek(input) != ';') { return; }
        advanceComment(input);
    }
}

void advanceComment(String input) {
    while (peek(input) != '\n' && peek(input) != '\0') { advanceChar(); }
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

typedef enum {
    PREC_NONE,
    PREC_EQUALITY,
    PREC_COMPARISON,
    PREC_CALL,
} Precedence;

typedef struct Compiler Compiler;

typedef void (*ParseFn)(Compiler* compiler);

typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

struct Compiler {
    Parser parser;
    AriaFunction function;
    Token locals[256];
    int local_count;
    Token function_name;
};

void advanceToken(Compiler* compiler);
bool check(Compiler* compiler, TokType type);
bool match(Compiler* compiler, TokType type);
void consume(Compiler* compiler, TokType type);
Chunk* currentChunk(Compiler* compiler);
void emitByte(Compiler* compiler, uint8_t byte);
void emitShort(Compiler* compiler, uint16_t value);
void emitName(Compiler* compiler, Token name);
int emitJump(Compiler* compiler, OP op);
void patchJump(Compiler* compiler, int offset);
void emitLoop(Compiler* compiler, int loop_start);
bool identifiersEqual(Compiler* compiler, Token a, Token b);
int addLocal(Compiler* compiler, Token name);
int resolveLocal(Compiler* compiler, Token name);
void expression(Compiler* compiler);
void statement(Compiler* compiler);
void parsePrecedence(Compiler* compiler, Precedence precedence);
void number(Compiler* compiler);
void variable(Compiler* compiler);
void grouping(Compiler* compiler);
void call(Compiler* compiler);
ParseRule* getRule(TokType type);
void binary(Compiler* compiler);
void parseType(Compiler* compiler);
void block(Compiler* compiler);
void varDeclaration(Compiler* compiler);
void ifStatement(Compiler* compiler);
void forStatement(Compiler* compiler);
void foreachStatement(Compiler* compiler);
void returnStatement(Compiler* compiler);
void funcDeclaration(Compiler* compiler);
void importDeclaration(Compiler* compiler);
const char* opcodeName(OP op);
uint16_t readShort(const Chunk* chunk, int offset);
void printOpcodes(Compiler* compiler);
void parse(const char* buf);

static ParseRule rules[TOK_COUNT] = {
    [TOK_LEFT_PAREN] = {grouping, call, PREC_CALL},
    [TOK_LESS] = {NULL, binary, PREC_COMPARISON},
    [TOK_EQUAL_EQUAL] = {NULL, binary, PREC_EQUALITY},
    [TOK_IDENTIFIER] = {variable, NULL, PREC_NONE},
    [TOK_NUM_LIT] = {number, NULL, PREC_NONE},
};

void advanceToken(Compiler* compiler) {
    compiler->parser.prev = compiler->parser.current;
    compiler->parser.current = scanToken(compiler->parser.input);
    assert(compiler->parser.current.type != TOK_ERROR);
}

bool check(Compiler* compiler, TokType type) {
    return compiler->parser.current.type == type;
}

bool match(Compiler* compiler, TokType type) {
    if (!check(compiler, type)) { return false; }
    advanceToken(compiler);
    return true;
}

void consume(Compiler* compiler, TokType type) {
    assert(check(compiler, type));
    advanceToken(compiler);
}

Chunk* currentChunk(Compiler* compiler) {
    return &compiler->function.chunk;
}

void emitByte(Compiler* compiler, uint8_t byte) {
    da_append(&currentChunk(compiler)->code, byte);
}

void emitShort(Compiler* compiler, uint16_t value) {
    emitByte(compiler, (value >> 8) & 0xff);
    emitByte(compiler, value & 0xff);
}

void emitName(Compiler* compiler, Token name) {
    assert(name.start >= 0 && name.start <= UINT16_MAX);
    assert(name.length >= 0 && name.length <= UINT16_MAX);
    emitShort(compiler, (uint16_t)name.start);
    emitShort(compiler, (uint16_t)name.length);
}

int emitJump(Compiler* compiler, OP op) {
    emitByte(compiler, op);
    emitByte(compiler, 0xff);
    emitByte(compiler, 0xff);
    return currentChunk(compiler)->code.count - 2;
}

void patchJump(Compiler* compiler, int offset) {
    const int jump = currentChunk(compiler)->code.count - offset - 2;
    assert(jump <= UINT16_MAX);
    currentChunk(compiler)->code.items[offset] = (jump >> 8) & 0xff;
    currentChunk(compiler)->code.items[offset + 1] = jump & 0xff;
}

void emitLoop(Compiler* compiler, int loop_start) {
    emitByte(compiler, OP_LOOP);
    const int offset = currentChunk(compiler)->code.count - loop_start + 2;
    assert(offset <= UINT16_MAX);
    emitShort(compiler, offset);
}

bool identifiersEqual(Compiler* compiler, Token a, Token b) {
    if (a.length != b.length) { return false; }
    return memcmp(compiler->parser.input.data + a.start, compiler->parser.input.data + b.start,
                  a.length) == 0;
}

int addLocal(Compiler* compiler, Token name) {
    assert(compiler->local_count < (int)(sizeof(compiler->locals) / sizeof(compiler->locals[0])));
    compiler->locals[compiler->local_count] = name;
    return compiler->local_count++;
}

int resolveLocal(Compiler* compiler, Token name) {
    for (int i = compiler->local_count - 1; i >= 0; i--) {
        if (identifiersEqual(compiler, name, compiler->locals[i])) { return i; }
    }

    return -1;
}

void number(Compiler* compiler) {
    char value[32] = {0};
    assert(compiler->parser.prev.length < (int)sizeof(value));
    memcpy(value, compiler->parser.input.data + compiler->parser.prev.start,
           compiler->parser.prev.length);

    const long number = strtol(value, NULL, 10);
    assert(number >= 0 && number <= UINT8_MAX);
    emitByte(compiler, OP_CONSTANT);
    emitByte(compiler, (uint8_t)number);
}

void variable(Compiler* compiler) {
    const int slot = resolveLocal(compiler, compiler->parser.prev);
    if (slot == -1) {
        emitByte(compiler, OP_GET_NAME);
        emitName(compiler, compiler->parser.prev);
        return;
    }

    emitByte(compiler, OP_GET_LOCAL);
    emitByte(compiler, (uint8_t)slot);
}

void grouping(Compiler* compiler) {
    expression(compiler);
    consume(compiler, TOK_RIGHT_PAREN);
}

void call(Compiler* compiler) {
    int argument_count = 0;
    if (!check(compiler, TOK_RIGHT_PAREN)) {
        do {
            expression(compiler);
            argument_count++;
        } while (match(compiler, TOK_COMMA));
    }
    assert(argument_count <= UINT8_MAX);
    consume(compiler, TOK_RIGHT_PAREN);

    emitByte(compiler, OP_CALL);
    emitByte(compiler, (uint8_t)argument_count);
}

void binary(Compiler* compiler) {
    const TokType operator_type = compiler->parser.prev.type;
    const ParseRule* rule = getRule(operator_type);
    parsePrecedence(compiler, (Precedence)(rule->precedence + 1));

    switch (operator_type) {
        case TOK_LESS: emitByte(compiler, OP_LESS); break;
        case TOK_EQUAL_EQUAL: emitByte(compiler, OP_EQUAL); break;
        default: assert(false); break;
    }
}

ParseRule* getRule(TokType type) {
    return &rules[type];
}

void parsePrecedence(Compiler* compiler, Precedence precedence) {
    advanceToken(compiler);
    ParseFn prefix = getRule(compiler->parser.prev.type)->prefix;
    assert(prefix != NULL);
    prefix(compiler);

    while (precedence <= getRule(compiler->parser.current.type)->precedence) {
        advanceToken(compiler);
        ParseFn infix = getRule(compiler->parser.prev.type)->infix;
        assert(infix != NULL);
        infix(compiler);
    }
}

void expression(Compiler* compiler) {
    parsePrecedence(compiler, PREC_EQUALITY);
}

void parseType(Compiler* compiler) {
    if (match(compiler, TOK_NUM)) { return; }

    consume(compiler, TOK_LIST);
    consume(compiler, TOK_LEFT_SQUACKET);
    consume(compiler, TOK_STR);
    consume(compiler, TOK_RIGHT_SQUACKET);
}

void block(Compiler* compiler) {
    while (!check(compiler, TOK_END) && !check(compiler, TOK_ELSE) && !check(compiler, TOK_EOF)) {
        statement(compiler);
    }
}

void varDeclaration(Compiler* compiler) {
    consume(compiler, TOK_IDENTIFIER);
    const int slot = addLocal(compiler, compiler->parser.prev);
    parseType(compiler);
    consume(compiler, TOK_EQUAL);
    expression(compiler);
    emitByte(compiler, OP_SET_LOCAL);
    emitByte(compiler, (uint8_t)slot);
}

void ifStatement(Compiler* compiler) {
    expression(compiler);
    consume(compiler, TOK_THEN);

    const int false_jump = emitJump(compiler, OP_JUMP_IF_FALSE);
    emitByte(compiler, OP_POP);
    block(compiler);

    const int end_jump = emitJump(compiler, OP_JUMP);
    patchJump(compiler, false_jump);
    emitByte(compiler, OP_POP);

    if (match(compiler, TOK_ELSE)) {
        if (match(compiler, TOK_IF)) {
            ifStatement(compiler);
            patchJump(compiler, end_jump);
            return;
        }

        block(compiler);
    }

    consume(compiler, TOK_END);
    patchJump(compiler, end_jump);
}

void forStatement(Compiler* compiler) {
    consume(compiler, TOK_IDENTIFIER);
    const int slot = addLocal(compiler, compiler->parser.prev);
    consume(compiler, TOK_EQUAL);
    expression(compiler);
    emitByte(compiler, OP_SET_LOCAL);
    emitByte(compiler, (uint8_t)slot);
    consume(compiler, TOK_TO);

    const int condition_start = currentChunk(compiler)->code.count;
    emitByte(compiler, OP_GET_LOCAL);
    emitByte(compiler, (uint8_t)slot);
    expression(compiler);
    emitByte(compiler, OP_LESS);
    const int exit_jump = emitJump(compiler, OP_JUMP_IF_FALSE);
    emitByte(compiler, OP_POP);

    const int body_jump = emitJump(compiler, OP_JUMP);
    const int increment_start = currentChunk(compiler)->code.count;
    emitByte(compiler, OP_GET_LOCAL);
    emitByte(compiler, (uint8_t)slot);
    if (match(compiler, TOK_STEP)) {
        expression(compiler);
    } else {
        emitByte(compiler, OP_CONSTANT);
        emitByte(compiler, 1);
    }
    emitByte(compiler, OP_ADD);
    emitByte(compiler, OP_SET_LOCAL);
    emitByte(compiler, (uint8_t)slot);
    emitLoop(compiler, condition_start);
    patchJump(compiler, body_jump);

    consume(compiler, TOK_THEN);
    block(compiler);
    emitLoop(compiler, increment_start);
    patchJump(compiler, exit_jump);
    emitByte(compiler, OP_POP);
    consume(compiler, TOK_END);
}

void foreachStatement(Compiler* compiler) {
    consume(compiler, TOK_IDENTIFIER);
    const int index_slot = addLocal(compiler, compiler->parser.prev);
    consume(compiler, TOK_COMMA);
    consume(compiler, TOK_IDENTIFIER);
    const int element_slot = addLocal(compiler, compiler->parser.prev);
    consume(compiler, TOK_IN);
    expression(compiler);
    consume(compiler, TOK_THEN);

    emitByte(compiler, OP_ITER_INIT);
    const int loop_start = currentChunk(compiler)->code.count;
    emitByte(compiler, OP_ITER_NEXT);
    emitByte(compiler, (uint8_t)index_slot);
    emitByte(compiler, (uint8_t)element_slot);
    const int exit_jump = currentChunk(compiler)->code.count;
    emitByte(compiler, 0xff);
    emitByte(compiler, 0xff);
    block(compiler);
    emitLoop(compiler, loop_start);
    patchJump(compiler, exit_jump);
    consume(compiler, TOK_END);
}

void returnStatement(Compiler* compiler) {
    expression(compiler);
    emitByte(compiler, OP_RETURN);
}

void statement(Compiler* compiler) {
    if (match(compiler, TOK_ELLIPSIS)) {
        return;
    } else if (match(compiler, TOK_VAR)) {
        varDeclaration(compiler);
    } else if (match(compiler, TOK_IF)) {
        ifStatement(compiler);
    } else if (match(compiler, TOK_FOR)) {
        forStatement(compiler);
    } else if (match(compiler, TOK_FOREACH)) {
        foreachStatement(compiler);
    } else if (match(compiler, TOK_RET)) {
        returnStatement(compiler);
    } else {
        expression(compiler);
        emitByte(compiler, OP_POP);
    }
}

void funcDeclaration(Compiler* compiler) {
    consume(compiler, TOK_IDENTIFIER);
    compiler->function = (AriaFunction){.name = compiler->parser.input.data + compiler->parser.prev.start};
    compiler->function_name = compiler->parser.prev;
    compiler->local_count = 0;

    consume(compiler, TOK_LEFT_PAREN);
    if (!check(compiler, TOK_RIGHT_PAREN)) {
        do {
            consume(compiler, TOK_IDENTIFIER);
            addLocal(compiler, compiler->parser.prev);
            compiler->function.arity++;
            parseType(compiler);
        } while (match(compiler, TOK_COMMA));
    }
    consume(compiler, TOK_RIGHT_PAREN);
    parseType(compiler);
    block(compiler);
    consume(compiler, TOK_END);
}

void importDeclaration(Compiler* compiler) {
    assert(check(compiler, TOK_STRING_LIT) || check(compiler, TOK_IDENTIFIER));
    advanceToken(compiler);
}

const char* opcodeName(OP op) {
    switch (op) {
        case OP_CONSTANT: return "OP_CONSTANT";
        case OP_GET_LOCAL: return "OP_GET_LOCAL";
        case OP_SET_LOCAL: return "OP_SET_LOCAL";
        case OP_GET_NAME: return "OP_GET_NAME";
        case OP_CALL: return "OP_CALL";
        case OP_LESS: return "OP_LESS";
        case OP_EQUAL: return "OP_EQUAL";
        case OP_ADD: return "OP_ADD";
        case OP_POP: return "OP_POP";
        case OP_JUMP_IF_FALSE: return "OP_JUMP_IF_FALSE";
        case OP_JUMP: return "OP_JUMP";
        case OP_LOOP: return "OP_LOOP";
        case OP_ITER_INIT: return "OP_ITER_INIT";
        case OP_ITER_NEXT: return "OP_ITER_NEXT";
        case OP_RETURN: return "OP_RETURN";
    }

    assert(false);
    return "";
}

uint16_t readShort(const Chunk* chunk, int offset) {
    return ((uint16_t)chunk->code.items[offset] << 8) | chunk->code.items[offset + 1];
}

void printOpcodes(Compiler* compiler) {
    const Chunk* chunk = &compiler->function.chunk;
    printf("== %.*s ==\n", compiler->function_name.length,
           compiler->parser.input.data + compiler->function_name.start);

    for (int offset = 0; offset < chunk->code.count;) {
        const OP op = chunk->code.items[offset++];
        printf("  %04d %-17s", offset - 1, opcodeName(op));

        switch (op) {
            case OP_CONSTANT: printf(" %d", chunk->code.items[offset++]); break;
            case OP_GET_LOCAL:
            case OP_SET_LOCAL: printf(" slot %d", chunk->code.items[offset++]); break;
            case OP_GET_NAME: {
                const uint16_t start = readShort(chunk, offset);
                const uint16_t length = readShort(chunk, offset + 2);
                offset += 4;
                printf(" %.*s", length, compiler->parser.input.data + start);
            } break;
            case OP_CALL: printf(" %d", chunk->code.items[offset++]); break;
            case OP_JUMP_IF_FALSE:
            case OP_JUMP: {
                const uint16_t jump = readShort(chunk, offset);
                offset += 2;
                printf(" -> %d", offset + jump);
            } break;
            case OP_LOOP: {
                const uint16_t jump = readShort(chunk, offset);
                offset += 2;
                printf(" -> %d", offset - jump);
            } break;
            case OP_ITER_NEXT: {
                const uint8_t index_slot = chunk->code.items[offset++];
                const uint8_t element_slot = chunk->code.items[offset++];
                const uint16_t jump = readShort(chunk, offset);
                offset += 2;
                printf(" slots %d, %d -> %d", index_slot, element_slot, offset + jump);
            } break;
            default: break;
        }
        printf("\n");
    }
}

void parse(const char* buf) {
    Compiler compiler = {0};
    pc = 0;
    compiler.parser.input = make_str_from_cstr(buf);
    compiler.parser.current = scanToken(compiler.parser.input);

    while (!check(&compiler, TOK_EOF)) {
        if (match(&compiler, TOK_IMPORT)) {
            importDeclaration(&compiler);
        } else if (match(&compiler, TOK_FUNC)) {
            funcDeclaration(&compiler);
        } else {
            assert(false);
        }
    }

    printOpcodes(&compiler);
    free(compiler.function.chunk.code.items);
    free(compiler.parser.input.data);
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

    (void)vm;

    FILE* fp = fopen(filepath, "r");
    if (fp == NULL) { return STATUS_FILENOTFOUND; }

    fseek(fp, 0, SEEK_END);
    size_t length = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* buf = malloc(length + 1);
    fread(buf, 1, length, fp);
    buf[length] = '\0';
    fclose(fp);

    parse(buf);
    free(buf);
    return STATUS_OK;
}

Status aria_call_func(AriaVM* vm, const char* func) {
    return STATUS_OK;
}
