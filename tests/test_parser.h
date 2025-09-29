#include "onetest.h"

#include "aria_lexer.h"
#include "aria_parser.h"

#define CREATE_LEXER(src) \
Aria_Lexer l = (Aria_Lexer){ \
        .source = src, \
        .pc = 0, \
        .tokens = bufferCreate(sizeof(Aria_Token), 8), \
        .buf_index = 0 \
    };

static int test_parseConst(void) { return 1; }
static int test_parseFor(void) { return 1; }
static int test_parseIf(void) { return 1; }
static int test_parsePrint(void) { return 1; }
static int test_parseReturn(void) {
    CREATE_LEXER("return 69;");

    Aria_Token tok = (Aria_Token){true, TOK_RETURN, 0, 6};
    bufferInsert(&l.tokens, &tok);

    tok = (Aria_Token){true, TOK_NUMBER, 7, 2};
    bufferInsert(&l.tokens, &tok);

    ASTNode node = parseReturn(&l);
    onetest_assert(node.type == AST_VALUE);
    onetest_assert(node.value == 69);

    return 0;
}
static int test_parseSwitch(void) { return 1; }
static int test_parseVar(void) { return 1; }
static int test_parseIdentifier(void) { return 1; }
static int test_parseExpression(void) { return 1; }
static int test_parseClass(void) { return 1; }
static int test_parseExport(void) { return 1; }
static int test_parseBlock(void) { return 1; }
static int test_parseFunc(void) { return 1; }
static int test_parseImport(void) { return 1; }
static int test_ariaParse(void) { return 1; }
static int test_createNode(void) { return 1; }
