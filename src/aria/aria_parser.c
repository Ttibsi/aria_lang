#include "aria_parser.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>

#include "nob.h"

#define MACRO_COMMENT(ignored)
#define parsingError(str, ...)                                                               \
    do {                                                                                     \
        fprintf(stderr, "%s:%d - " str "\n", __func__, __LINE__ __VA_OPT__(, ) __VA_ARGS__); \
        MACRO_COMMENT(asm("brk #0x1");)                                                      \
        assert(0);                                                                           \
    } while (0)

binding_t prefixBindingPower(const TokenType* tkn) {
    switch (*tkn) {
        case TOK_BANG:
            [[fallthrough]];
        case TOK_MINUS:
            return 8;
            break;
        default:
            return 0;
    }

    return 0;
}

binding_t infixBindingPower(const TokenType* tkn) {
    switch (*tkn) {
        case TOK_DOT:
            [[fallthrough]];
        case TOK_LEFT_PAREN:
            [[fallthrough]];
        case TOK_RIGHT_PAREN:
            return 9;
        case TOK_STAR:
            [[fallthrough]];
        case TOK_SLASH:
            return 7;
        case TOK_PLUS:
            [[fallthrough]];
        case TOK_MINUS:
            return 6;
        case TOK_GREATER:
            [[fallthrough]];
        case TOK_GREATER_EQUAL:
            [[fallthrough]];
        case TOK_LESS:
            [[fallthrough]];
        case TOK_LESS_EQUAL:
            return 5;
        case TOK_EQUAL_EQUAL:
            [[fallthrough]];
        case TOK_BANG_EQUAL:
            return 4;
        case TOK_AND:
            return 3;
        case TOK_OR:
            return 2;
        case TOK_EQUAL:
            return 1;
        default:
            return 0;
    };

    return 0;
}

ASTNode parseBlock(AriaLexer* L) {
    ASTNode node = ariaCreateNode(AST_BLOCK);

    while (!(check(L, TOK_END) || check(L, TOK_ELSE))) {
        // ellipses aren't parsed but force the end of the block
        if (match(L, TOK_ELLIPSIS)) { break; }

        ASTNode stmt = parseStatement(L);
        nob_da_append(&node.block, stmt);
    }

    return node;
}

ASTNode* parseArg(AriaLexer* L) {
    ASTNode* node = malloc(sizeof(ASTNode));

    // name
    const AriaToken* tok = &L->items[L->index];
    Nob_String_Builder arg_name = {0};
    nob_sb_append_buf(&arg_name, &L->source[tok->start], tok->len);
    nob_sb_append_null(&arg_name);
    node->arg.name = arg_name.items;
    advance(L);

    // type
    if (!checkType(L)) { parsingError("Type not defined for argument"); }
    node->type = L->items[L->index].type;
    advance(L);

    if (node->type == TOK_LIST || node->type == TOK_MAP) {
        if (!match(L, TOK_LEFT_SQUACKET)) {
            parsingError("Container type doesn't define contents types");
        }
        if (!checkType(L)) { parsingError("Type not defined for container"); }

        node->arg.inner_type_1 = L->items[L->index].type;
        advance(L);

        if (node->arg.type == TOK_MAP) {
            if (!match(L, TOK_COMMA)) {
                parsingError("Map requires two types separated by commas");
            }
            node->arg.inner_type_2 = L->items[L->index].type;
        }

        if (!match(L, TOK_RIGHT_SQUACKET)) { parsingError("Malformed container type"); }
    }

    return node;
}

ASTNode parseFunc(AriaLexer* L) {
    if (!match(L, TOK_FUNC)) { parsingError("parseFunc called with incorrect token\n"); }
    ASTNode funcNode = ariaCreateNode(AST_FUNC);

    // func name
    const AriaToken* tok = &L->items[L->index];
    Nob_String_Builder name = {0};
    nob_sb_append_buf(&name, &L->source[tok->start], tok->len);
    nob_sb_append_null(&name);
    funcNode.func.name = name.items;
    advance(L);

    // arguments
    // All default to 0 values, akin to `AriaToken args[i] = {0}`
    if (!match(L, TOK_LEFT_PAREN)) { parsingError("Function name not followed by open bracket\n"); }
    int args_idx = 0;
    while (!check(L, TOK_RIGHT_PAREN)) {
        if (!check(L, TOK_IDENTIFIER)) { parsingError("function args contain non-identifiers\n"); }
        if (args_idx >= param_count) { parsingError("Function has too many arguments\n"); }

        funcNode.func.args[args_idx] = parseArg(L);
        args_idx++;
        if (check(L, TOK_COMMA)) { advance(L); }  // Skip commas
    }
    advance(L);

    // return type
    if (!checkType(L)) { parsingError("Function return type not specified\n"); }
    funcNode.func.ret_type = L->items[L->index].type;
    advance(L);

    // body
    ASTNode* body = malloc(sizeof(ASTNode));
    *body = parseBlock(L);
    funcNode.func.body = body;

    match(L, TOK_END);
    return funcNode;
}

ASTNode parseFuncCall(AriaLexer* L) {
    ASTNode funcNode = ariaCreateNode(AST_CALL);

    // func name
    const AriaToken* tok = &L->items[L->index];
    Nob_String_Builder name = {0};
    nob_sb_append_buf(&name, &L->source[tok->start], tok->len);
    nob_sb_append_null(&name);
    funcNode.func.name = name.items;
    advance(L);

    // arguments
    if (!match(L, TOK_LEFT_PAREN)) { parsingError("Function name not followed by open bracket\n"); }
    int args_idx = 0;
    while (!check(L, TOK_RIGHT_PAREN)) {
        if (!check(L, TOK_IDENTIFIER)) { parsingError("function args contain non-identifiers\n"); }
        if (args_idx >= param_count) { parsingError("Function has too many arguments\n"); }

        const AriaToken* tok = &L->items[L->index];
        Nob_String_Builder arg_name = {0};
        nob_sb_append_buf(&arg_name, &L->source[tok->start], tok->len);
        nob_sb_append_null(&arg_name);
        funcNode.funcCall.args[args_idx] = arg_name.items;
        advance(L);

        args_idx++;
        if (check(L, TOK_COMMA)) { advance(L); }  // Skip commas
    }
    advance(L);

    return funcNode;
}

ASTNode parseExpression(AriaLexer* L, const binding_t min_bp) {
    ASTNode lhs = {};

    switch (getCurrTokenType(L)) {
        case TOK_NUM_LIT:
            lhs = ariaCreateNode(AST_NUM_LIT);
            lhs.num_literal = getTokenNumber(L, L->index);
            break;
        case TOK_LEFT_PAREN:
            advance(L);
            lhs = parseExpression(L, 0);
            break;
        case TOK_IDENTIFIER:
            if (L->items[L->index + 1].type == TOK_LEFT_PAREN) {
                lhs = parseFuncCall(L);
            } else {
                lhs = parseIdentifier(L);
            }
            break;
        case TOK_ELLIPSIS:
            [[fallthrough]];
        default:
            parsingError("Unknown token found (parseExpression LHS): %d", getCurrTokenType(L));
            return lhs;
    };

    const AriaToken* next = &L->items[L->index + 1];
    if (next->type == TOK_EOF) {
        parsingError("EOF reached when parsing an expression");
    } else if (next->type == TOK_RIGHT_PAREN || isKeyword(next->type)) {
        return lhs;
    }

    while (true) {
        advance(L);
        const TokenType tok_type = getCurrTokenType(L);
        if (tok_type == TOK_RIGHT_PAREN || isKeyword(tok_type)) { break; }

        const binding_t bp = infixBindingPower(&tok_type);
        // If the token is anything other than expected, we'll get a bp of 0
        if (bp == 0) {
            L->index--;
            return lhs;
        }

        if (bp < min_bp) { break; }
        advance(L);

        ASTNode rhs = parseExpression(L, bp + 1);

        ASTNode* lhs_ptr = malloc(sizeof(ASTNode));
        ASTNode* rhs_ptr = malloc(sizeof(ASTNode));
        *lhs_ptr = lhs;
        *rhs_ptr = rhs;

        lhs = ariaCreateNode(AST_EXPR);
        lhs.expr.op = tok_type;
        lhs.expr.lhs = lhs_ptr;
        lhs.expr.rhs = rhs_ptr;
    }

    return lhs;
}

ASTNode parseFor(AriaLexer* L) {
    if (!(match(L, TOK_FOR))) { parsingError("for statement invalid"); }
    ASTNode node = ariaCreateNode(AST_FOR);

    node.For.var = malloc(sizeof(ASTNode));
    *node.For.var = parseIdentifier(L);
    advance(L);
    if (!(match(L, TOK_EQUAL))) { parsingError("for loop assignment invalid"); }

    node.For.start = malloc(sizeof(ASTNode));
    *node.For.start = parseExpression(L, 0);
    advance(L);  // to TOK_TO
    if (!(match(L, TOK_TO))) { parsingError("for loop startpoint invalid"); }

    node.For.stop = malloc(sizeof(ASTNode));
    *node.For.stop = parseExpression(L, 0);
    advance(L);

    if (match(L, TOK_STEP)) {
        node.For.step = malloc(sizeof(ASTNode));
        *node.For.step = parseExpression(L, 0);
        advance(L);
    }

    if (!(match(L, TOK_THEN))) { parsingError("for loop block invalid"); }
    node.For.block = malloc(sizeof(ASTNode));
    *node.For.block = parseBlock(L);

    match(L, TOK_END);
    return node;
}

ASTNode parseForEach(AriaLexer* L) {
    if (!(match(L, TOK_FOREACH))) { parsingError("foreach statement invalid"); }
    ASTNode node = ariaCreateNode(AST_FOREACH);

    node.ForEach.first_var = malloc(sizeof(ASTNode));
    *node.ForEach.first_var = parseIdentifier(L);
    advance(L);
    match(L, TOK_COMMA);

    node.ForEach.sec_var = malloc(sizeof(ASTNode));
    *node.ForEach.sec_var = parseIdentifier(L);
    advance(L);
    match(L, TOK_IN);

    node.ForEach.container = malloc(sizeof(ASTNode));
    *node.ForEach.container = parseIdentifier(L);
    advance(L);
    match(L, TOK_THEN);

    node.ForEach.block = malloc(sizeof(ASTNode));
    *node.ForEach.block = parseBlock(L);
    match(L, TOK_END);

    return node;
}

// NOTE: Intentionally doesn't swallow TOK_IDENTIFIER
ASTNode parseIdentifier(AriaLexer* L) {
    return (ASTNode){.type = AST_IDENT, .identifier = getTokenString(L, L->index)};
}

ASTNode parseIf(AriaLexer* L) {
    if (!(match(L, TOK_IF))) { parsingError("If statement invalid"); }

    ASTNode ifNode = ariaCreateNode(AST_IF);
    ifNode.If.cond = malloc(sizeof(ASTNode));
    *ifNode.If.cond = parseExpression(L, 0);
    if (!(match(L, TOK_THEN))) { parsingError("No THEN keyword found"); }

    ifNode.If.block = malloc(sizeof(ASTNode));
    *ifNode.If.block = parseBlock(L);

    if (match(L, TOK_ELSE)) {
        ifNode.If.elseBlock = malloc(sizeof(ASTNode));

        if (check(L, TOK_IF)) {
            *ifNode.If.elseBlock = parseIf(L);
        } else {
            *ifNode.If.elseBlock = parseBlock(L);
        }
    }

    // not always at a TOK_END if we have ELSE/IFs, so we only advance if we're at one
    match(L, TOK_END);
    return ifNode;
}

ASTNode parseImport(AriaLexer* L) {
    ASTNode node = ariaCreateNode(AST_IMPORT);
    advance(L);
    if (!(match(L, TOK_IDENTIFIER) || match(L, TOK_STRING_LIT))) {
        parsingError("Import statement invalid");
    }

    if (match(L, TOK_STRING_LIT)) { node.import.local_file = true; }

    const AriaToken* tok = &L->items[L->index];
    Nob_String_Builder name = {0};
    nob_sb_append_buf(&name, &L->source[tok->start], tok->len);
    nob_sb_append_null(&name);
    node.import.name = name.items;

    return node;
}

ASTNode parseReturn(AriaLexer* L) {
    ASTNode node = ariaCreateNode(AST_RETURN);
    advance(L);

    ASTNode* expr = malloc(sizeof(ASTNode));
    *expr = parseExpression(L, 0);

    if (expr->type == AST_EXPR) {
        node.ret.expr = expr;
    } else {
        node.ret.expr = malloc(sizeof(ASTNode));
        node.ret.expr->type = AST_EXPR;
        node.ret.expr->expr.lhs = expr;
        node.ret.expr->expr.rhs = NULL;
        node.ret.expr->expr.op = expr->type;
        advance(L);
    }

    match(L, TOK_SEMICOLON);  // consume semicolon if needed
    return node;
}

ASTNode parseMethodCall(AriaLexer* L) {
    if (!check(L, TOK_IDENTIFIER)) { parsingError("Can't parse method"); }
    ASTNode node = ariaCreateNode(AST_METHOD_CALL);

    advance(L);

    if (!match(L, TOK_DOT)) { parsingError("No dot found in method call"); }

    node.methodCall.method = malloc(sizeof(ASTNode));
    *node.methodCall.method = parseFuncCall(L);

    return node;
}

ASTNode parseMethodCallOrAttr(AriaLexer* L) {
    // TOK_IDENT TOK_DOT TOK_IDENT TOK_LEFT_PARAM
    if (L->items[L->index + 3].type == TOK_LEFT_PAREN) { return parseMethodCall(L); }

    ASTNode object_ident = parseIdentifier(L);
    match(L, TOK_IDENTIFIER);
    match(L, TOK_DOT);

    ASTNode assign = parseAssignment(L);
    assign.assign.object_ident = malloc(sizeof(ASTNode));
    *assign.assign.object_ident = object_ident;

    return assign;
}

ASTNode parseAssignment(AriaLexer* L) {
    if (!check(L, TOK_IDENTIFIER)) {
        parsingError("Assignment statement not starting with identifier");
    }
    ASTNode node = ariaCreateNode(AST_ASSIGN);

    node.assign.ident = malloc(sizeof(ASTNode));
    *node.assign.ident = parseIdentifier(L);
    match(L, TOK_IDENTIFIER);

    if (!match(L, TOK_EQUAL)) { parsingError("assignment expression has no = token"); }

    node.assign.expr = malloc(sizeof(ASTNode));
    *node.assign.expr = parseExpression(L, 0);

    advance(L);
    return node;
}

ASTNode parseStatement(AriaLexer* L) {
    AriaToken* tkn = &L->items[L->index];
    switch (tkn->type) {
        case TOK_FOR:
            return parseFor(L);
            break;
        case TOK_FOREACH:
            return parseForEach(L);
            break;
        case TOK_IF:
            return parseIf(L);
            advance(L);  // TOK_END
            break;
        case TOK_RET:
            return parseReturn(L);
            break;
        case TOK_VAR:
            return parseVar(L);
            break;
        case TOK_IDENTIFIER:
            if (L->items[L->index + 1].type == TOK_DOT) {
                return parseMethodCallOrAttr(L);
            } else if (L->items[L->index + 1].type == TOK_EQUAL) {
                return parseAssignment(L);
            } else if (L->items[L->index + 1].type == TOK_LEFT_PAREN) {
                return parseFuncCall(L);
            }
            break;
        default:
            parsingError("Incorrect token found in parseStatement (default)\n");
            break;
    };

    parsingError("Incorrect token found in parseStatement\n");
    return ERR_NODE;
}

ASTNode parseType(AriaLexer* L) {
    if (!(match(L, TOK_TYPE))) { parsingError("var statement invalid"); }
    ASTNode node = ariaCreateNode(AST_TYPE);

    // type name
    const AriaToken* tok = &L->items[L->index];
    Nob_String_Builder name = {0};
    nob_sb_append_buf(&name, &L->source[tok->start], tok->len);
    nob_sb_append_null(&name);
    node.Type.name = name.items;
    advance(L);

    // attributes
    while (check(L, TOK_VAR)) { nob_da_append(&node.Type.Vars, parseVar(L)); }

    // methods
    while (check(L, TOK_FUNC)) { nob_da_append(&node.Type.Methods, parseFunc(L)); }

    match(L, TOK_END);
    return node;
}

ASTNode parseVar(AriaLexer* L) {
    if (!(match(L, TOK_VAR))) { parsingError("var statement invalid"); }
    ASTNode node = ariaCreateNode(AST_VAR);

    // variable name
    const AriaToken* tok = &L->items[L->index];
    Nob_String_Builder name = {0};
    nob_sb_append_buf(&name, &L->source[tok->start], tok->len);
    nob_sb_append_null(&name);
    node.var.name = name.items;
    advance(L);

    // variable type
    if (!checkType(L)) { parsingError("Type not defined for variable"); }
    node.var.ret_type = L->items[L->index].type;
    advance(L);

    // value
    if (match(L, TOK_EQUAL)) {
        if (check(L, TOK_STRING_LIT)) {
            node.var.value = malloc(sizeof(ASTNode));
            node.var.value->type = AST_STR_LIT;
            node.var.value->string_literal = getTokenString(L, L->index);
        } else if (check(L, TOK_NUM_LIT)) {
            node.var.value = malloc(sizeof(ASTNode));
            node.var.value->type = AST_NUM_LIT;
            node.var.value->num_literal = getTokenNumber(L, L->index);
        } else if (check(L, TOK_CHAR_LIT)) {
            node.var.value = malloc(sizeof(ASTNode));
            node.var.value->type = AST_CHAR_LIT;
            node.var.value->char_literal = getTokenChar(L, L->index);
        }

        advance(L);
    }

    return node;
}

ASTNode ariaCreateNode(const NodeType type) {
    switch (type) {
        case AST_ARG:
            return (ASTNode){.type = type,
                             .arg = {
                                 .name = NULL,
                             }};

        case AST_ASSIGN:
            return (ASTNode){.type = type,
                             .assign = {.ident = NULL, .object_ident = NULL, .expr = NULL}};

        case AST_BLOCK:
            [[fallthrough]];
        case AST_MODULE:
            return (ASTNode){.type = type, .block = {.name = NULL}};

        case AST_CALL:
            return (ASTNode){.type = type, .funcCall = {.name = NULL}};

        case AST_CHAR_LIT:
            return (ASTNode){.type = type, .char_literal = '\0'};

        case AST_EXPR:
            return (ASTNode){.type = type, .expr = {.lhs = NULL, .rhs = NULL}};

        case AST_FOR:
            return (ASTNode){
                .type = type,
                .For = {.var = NULL, .start = NULL, .stop = NULL, .step = NULL, .block = NULL}};

        case AST_FOREACH:
            return (ASTNode){
                .type = type,
                .ForEach = {.first_var = NULL, .sec_var = NULL, .container = NULL, .block = NULL}};

        case AST_FUNC:
            return (ASTNode){.type = type, .func = {.name = NULL, .body = NULL}};

        case AST_IF:
            return (ASTNode){.type = type, .If = {.cond = NULL, .block = NULL, .elseBlock = NULL}};

        case AST_IMPORT:
            return (ASTNode){.type = type, .import = {.local_file = false, .name = NULL}};

        case AST_METHOD_CALL:
            return (ASTNode){.type = type, .methodCall = {.object = NULL, .method = NULL}};

        case AST_NUM_LIT:
            return (ASTNode){.type = type, .num_literal = 0};

        case AST_RETURN:
            return (ASTNode){.type = type};

        case AST_STR_LIT:
            return (ASTNode){.type = type, .string_literal = ""};

        case AST_TYPE:
            return (ASTNode){.type = type, .Type = {.name = NULL}};

        case AST_VAR:
            return (ASTNode){.type = type, .var = {.name = NULL, .value = NULL}};

        case NODE_COUNT:
            [[fallthrough]];
        default:
            NOB_UNREACHABLE("Unhandled type in ariaCreateNode");
    }
}

ASTNode ariaParse(AriaLexer* L, char* mod_name) {
    ASTNode module = ariaCreateNode(AST_MODULE);
    module.block.name = mod_name;

    while (!check(L, TOK_EOF)) {
        ASTNode inner = ERR_NODE;

        switch (getCurrTokenType(L)) {
            case TOK_FUNC:
                inner = parseFunc(L);
                break;

            case TOK_IMPORT:
                inner = parseImport(L);
                break;

            case TOK_TYPE:
                inner = parseType(L);
                break;

            case TOK_ERROR:
                NOB_UNREACHABLE("ERROR node at top level in ariaParse");
                break;

            default:
                NOB_UNREACHABLE("Invalid node at top level in ariaParse");
        };

        nob_da_append(&module.block, inner);
    }

    printf("Module %s parsing complete\n", mod_name);
    return module;
}

// Recursive impl for printing nodes
void printASTNode(const ASTNode* n, int offset) {
    printf("%*s\n", offset, "");

    switch (n->type) {
        case AST_ARG:
            printf("@arg[%s]\n", n->arg.name);
            break;

        case AST_MODULE:
            printf("@module[\n");
            printf("%*sModule name:%s\n", offset + 2, "", n->block.name);
            nob_da_foreach(ASTNode, inner, &n->block) { printASTNode(inner, offset + 2); }
            printf("%*s]\n", offset, "");
            break;

        case AST_BLOCK:
            printf("@block[\n");
            nob_da_foreach(ASTNode, inner, &n->block) { printASTNode(inner, offset + 2); }
            printf("%*s]\n", offset, "");
            break;

        case AST_ERR:
            printf("@error\n");
            break;

        case AST_EXPR:
            printf("@expr[");
            printASTNode(n->expr.lhs, offset + 2);

            if (n->expr.rhs != NULL) {
                printf("%*sop: [%d]\n", offset + 2, "", n->expr.op);
                printASTNode(n->expr.rhs, offset + 2);
            };
            printf("%*s]\n", offset, "");
            break;

        case AST_FUNC:
            printf("@func[\n");
            printf("%*sfunc name: %s\n", offset + 2, "", n->func.name);

            // TODO: args
            // TODO: ret type
            // TODO: block

            printf("%*s]\n", offset, "");
            break;

        case AST_IMPORT:
            printf("@import[%s]\n", n->import.name);
            break;

        case AST_RETURN:
            printf("@ret[\n");
            printASTNode(n->ret.expr, offset + 2);
            printf("%*s]\n", offset, "");
            break;

        case NODE_COUNT:
            [[fallthrough]];
        default:
            break;
    }
}

void printAst(const ASTNode* root) {
    printf("=== AST ===\n");
    printASTNode(root, 0);
    // nob_da_foreach(ASTNode, node, &root->block) {
    //     printASTNode(node, 2);
    // }
}
