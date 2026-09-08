#include "aria.h"

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
    int len;

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
    const char* start;
    int length;
    int line;
} Token;

typedef struct {
    Token prev;
    Token current;
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
String make_str_from_cstr(const char* data) {
    const int len = strlen(data);
    String str = {0};
    str.data = malloc(sizeof(char) * len);
    memcpy(str.data, data, len);
    str.len = len;

    uint32_t hash = 2166136261u;
    for (int i = 0; i < len; i++) {
        hash ^= (uint8_t)data[i];
        hash *= 16777619;
    }
    str.hash = hash;

    return str;
}

bool str_cmp(String* a, String* b) {
    if (a->len != b->len) { return false; }
    for (int i = 0; i < a->len; i++) {
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

void parse(const char* buf) {
    // While the current token isn't EOF, we pass the current token
    // into the top of the RD parser. The RD emits bytes straight
    // into the chunk to construct the AriaFunction obj
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

Status aria_call_func(AriaVM* vm, const char* func) {}
