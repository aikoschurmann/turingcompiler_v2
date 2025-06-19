#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pratt_parse.h"


const char *astnode_type_to_string(AstNodeType type)
{
    switch (type) {
        case AST_LITERAL:      return "AST_LITERAL";
        case AST_VARIABLE:     return "AST_VARIABLE";
        case AST_UNARY_OP:     return "AST_UNARY_OP";
        case AST_BINARY_OP:    return "AST_BINARY_OP";
        case AST_IF:           return "AST_IF";
        case AST_WHILE:        return "AST_WHILE";
        case AST_BLOCK:        return "AST_BLOCK";
        case AST_FUNCTION:     return "AST_FUNCTION";
        case AST_DECLARATION:  return "AST_DECLARATION";
        case AST_ASSIGNMENT:   return "AST_ASSIGNMENT";
        case AST_RETURN:       return "AST_RETURN";
        case AST_CALL:         return "AST_CALL";
        default:               return "UNKNOWN_AST_NODE";
    }
}

char *binaryop_to_string(BinaryOp op)
{
    switch (op) {
        case OP_ADD: return "+";
        case OP_SUB: return "-";
        case OP_MUL: return "*";
        case OP_DIV: return "/";
        case OP_LT:  return "<";
        case OP_EQ:  return "==";
        case OP_GT:  return ">";
        case OP_LEQ: return "<=";
        case OP_GEQ: return ">=";
        case OP_NEQ: return "!=";
        default:     return "UNKNOWN_BINARY_OP";
    }
}

// Error-reporting and exit
void parse_error(const Parser *parser,
                        TokenType expected,
                        const Token *actual)
{
    fprintf(stderr,
            "%s:%zu:%zu: parse error: expected <%s> but got <%s> (%d)\n",
            parser->filename,
            actual->line,
            actual->column,
            token_type_to_string(expected),
            token_type_to_string(actual->type),
            actual->type);
    exit(EXIT_FAILURE);
}

Token *current_token(Parser *p) {
    if (p->current >= p->end) {
        static Token eof = { .type = TOKEN_EOF };
        return &eof;
    }
    return p->tokens.data[p->current];
}

Token *peek(Parser *p, size_t offset) {
    size_t idx = p->current + offset;
    return (idx < p->end) ? p->tokens.data[idx] : NULL;
}

Token *consume(Parser *p, TokenType expected, const char *value) {
    Token *tok = current_token(p);
    if (tok->type != expected) parse_error(p, expected, tok);
    if (value && strcmp(tok->value, value) != 0) parse_error(p, expected, tok);
    p->current++;
    return tok;
}

// Memory management
AstBlock *ast_block_create(void)
{
    AstBlock *block = malloc(sizeof(*block));
    if (!block) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    block->statements = NULL;
    block->count      = 0;
    block->capacity   = 0;
    return block;
}

void ast_block_push(AstBlock *block, AstNode *stmt)
{
    if (block->count == block->capacity) {
        size_t newcap = block->capacity ? block->capacity * 2 : 4;
        block->statements = realloc(block->statements,
                                    newcap * sizeof(*block->statements));
        if (!block->statements) {
            perror("realloc");
            exit(EXIT_FAILURE);
        }
        block->capacity = newcap;
    }
    block->statements[block->count++] = stmt;
}

void free_ast_node(AstNode *node)
{
    if (!node) return;

    switch (node->type) {
    case AST_LITERAL:
        // no heap data
        break;

    case AST_VARIABLE:
        free(node->data.variable.identifier);
        break;

    case AST_UNARY_OP:
        free_ast_node(node->data.unary.operand);
        break;

    case AST_BINARY_OP:
        free_ast_node(node->data.binary.left);
        free_ast_node(node->data.binary.right);
        break;

    case AST_IF:
        free_ast_node(node->data.if_stmt.condition);
        free_ast_node((AstNode *)node->data.if_stmt.then_block);
        free_ast_node((AstNode *)node->data.if_stmt.else_block);
        break;

    case AST_WHILE:
        free_ast_node(node->data.while_loop.condition);
        free_ast_node((AstNode *)node->data.while_loop.body);
        break;

    case AST_BLOCK:
        for (size_t i = 0; i < node->data.block.count; i++)
            free_ast_node(node->data.block.statements[i]);
        free(node->data.block.statements);
        break;

    case AST_FUNCTION: {
        AstFunction *fn = &node->data.function;
        free(fn->name);
        free_ast_node((AstNode *)fn->body);
        for (size_t i = 0; i < fn->params->count; i++) {
            free(fn->params->params[i]->identifier);
            free(fn->params->params[i]);
        }
        free(fn->params->params);
        break;
    }

    case AST_DECLARATION:
        free(node->data.declaration.variable->data.variable.identifier);
        free(node->data.declaration.variable);
        free_ast_node(node->data.declaration.value);
        break;

    case AST_ASSIGNMENT:
        free(node->data.assignment.variable->data.variable.identifier);
        free(node->data.assignment.variable);
        free_ast_node(node->data.assignment.value);
        break;

    case AST_RETURN:
        free_ast_node(node->data.return_stmt.expression);
        break;

    case AST_CALL: {
        AstCall *call = &node->data.call;
        free(call->callee->identifier);
        free(call->callee);
        for (size_t i = 0; i < call->args->count; i++)
            free_ast_node(call->args->arguments[i]);
        free(call->args->arguments);
        break;
    }

    default:
        break;
    }
    free(node);
}

AstNode *ast_create_node(AstNodeType type)
{
    AstNode *node = malloc(sizeof(*node));
    if (!node) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    node->type = type;
    memset(&node->data, 0, sizeof(node->data)); // Zero-initialize data
    return node;
}

// Parser lifecycle
Parser *parser_create(TokenArray tokens, const char *filename)
{
    Parser *p = malloc(sizeof(*p));
    if (!p) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    p->tokens   = tokens;
    p->start    = 0;                 // cover the whole stream
    p->end      = tokens.size;
    p->current  = p->start;
    p->filename = strdup(filename);
    return p;
}

Parser parser_slice(const Parser *orig,
                    size_t slice_start,
                    size_t slice_end)
{
    Parser slice = *orig;       // shallow copy
    slice.start   = slice_start;
    slice.current = slice_start;
    slice.end     = slice_end;
    return slice;
}


void parser_free(Parser *parser)
{
    if (!parser) return;
    token_array_free(&parser->tokens);
    free(parser->filename);
    free(parser);
}

// AST printing
static void print_indent(int level)
{
    for (int i = 0; i < level; i++) {
        fputs("  ", stdout);
    }
}

void print_ast(AstNode *node, int indent)
{
    if (!node) return;
    print_indent(indent);
    switch (node->type) {
    case AST_BLOCK:
        puts("Block");
        for (size_t i = 0; i < node->data.block.count; i++)
            print_ast(node->data.block.statements[i], indent + 1);
        break;

    case AST_VARIABLE:
        printf("Variable: %s\n", node->data.variable.identifier);
        break;

    case AST_LITERAL:
        printf("IntLiteral: %d\n", node->data.literal.value);
        break;

    case AST_BINARY_OP:
        printf("BinaryOp: %s\n",
               binaryop_to_string(node->data.binary.op));
        print_ast(node->data.binary.left, indent + 1);
        print_ast(node->data.binary.right, indent + 1);
        break;

    case AST_UNARY_OP:
        printf("UnaryOp: %c\n", node->data.unary.op);
        print_ast(node->data.unary.operand, indent + 1);
        break;

    case AST_DECLARATION:
        puts("Declaration");
        print_ast(node->data.declaration.variable, indent + 1);
        print_ast(node->data.declaration.value, indent + 1);
        break;

    case AST_ASSIGNMENT:
        printf("Assignment: %s\n",
               node->data.assignment.variable->data.variable.identifier);
        print_ast(node->data.assignment.value, indent + 1);
        break;

    case AST_CALL:
        printf("Call: %s\n", node->data.call.callee->identifier);
        for (size_t i = 0; i < node->data.call.args->count; i++)
            print_ast(node->data.call.args->arguments[i], indent + 1);
        break;
    
    case AST_IF:
        printf("IfStatement\n");
        print_indent(indent + 1);
        printf("Condition:\n");
        print_ast(node->data.if_stmt.condition, indent + 2);
        print_indent(indent + 1);
        printf("ThenBlock:\n");
        print_ast((AstNode *)node->data.if_stmt.then_block, indent + 2);
        if (node->data.if_stmt.else_block) {
            print_indent(indent + 1);
            printf("ElseBlock:\n");
            print_ast((AstNode *)node->data.if_stmt.else_block, indent + 2);
        }
        break;

    default:
        printf("<Unknown AST node: %s>\n", astnode_type_to_string(node->type));
        break;
    }
}

size_t parser_find_first_token(Parser *p, TokenType type)
{   
    for (size_t i = p->current; i < p->end; i++) {
        if (p->tokens.data[i]->type == type) {
            return i;
        }
    }
    parse_error(p, type, p->tokens.data[p->end - 1]);
}

static size_t parser_find_matching(Parser *p, TokenType open, TokenType close) {
    size_t depth = 1;
    for (size_t i = p->current; i < p->end; i++) {
        TokenType t = p->tokens.data[i]->type;
        if (t == open) {
            depth++;
        } else if (t == close) {
            if (--depth == 0) {
                return i;
            }
        }
    }
    // No matching brace found → error
    parse_error(p, close, p->tokens.data[p->end - 1]);
    return p->end;  // unreachable
}


AstNode *parse_expression(Parser *p) {
    return parse_expression_pratt(p, 0);
}


AstNode *parse_declaration(Parser *p)
{
    consume(p, TOKEN_DEFINE, NULL);
    Token *var = consume(p, TOKEN_IDENTIFIER, NULL);

    AstNode *var_node = ast_create_node(AST_VARIABLE);
    var_node->data.variable.identifier = strdup(var->value);

    consume(p, TOKEN_OPERATOR, "=");

    AstNode *decl = ast_create_node(AST_DECLARATION);
    decl->data.declaration.variable = var_node;
    decl->data.declaration.value    = parse_expression(p);
    
    consume(p, TOKEN_END_OF_LINE, NULL);
    return decl;
}

AstNode *parse_if_statement(Parser *p)
{
    consume(p, TOKEN_IF, NULL);
    consume(p, TOKEN_PAREN_OPEN, NULL);
    

    AstNode *condition = parse_expression(p);

    consume(p, TOKEN_PAREN_CLOSE, NULL);
    consume(p, TOKEN_BRACE_OPEN, NULL);

    // Parse the 'then' block
    size_t then_end = parser_find_matching(p, TOKEN_BRACE_OPEN, TOKEN_BRACE_CLOSE);
    Parser then_parser = parser_slice(p, p->current, then_end);
    
    AstNode *then_block = parse(&then_parser);
    p->current = then_end;
    consume(p, TOKEN_BRACE_CLOSE, NULL);

    // Create the 'if' node
    AstNode *if_node = ast_create_node(AST_IF);
    if_node->data.if_stmt.condition = condition;
    if_node->data.if_stmt.then_block = (AstBlock *)then_block;
    if_node->data.if_stmt.else_block = NULL;

    // Check for optional 'else' block
    if (current_token(p)->type == TOKEN_ELSE) {
        consume(p, TOKEN_ELSE, NULL);
        consume(p, TOKEN_BRACE_OPEN, NULL);

        size_t else_end = parser_find_matching(p, TOKEN_BRACE_OPEN, TOKEN_BRACE_CLOSE);
        Parser else_parser = parser_slice(p, p->current, else_end);
        AstNode *else_block = parse(&else_parser);
        p->current = else_end;
        consume(p, TOKEN_BRACE_CLOSE, NULL);


        if_node->data.if_stmt.else_block = (AstBlock *)else_block;
    }

    return if_node;
}

AstNode *parse_statement(Parser *p)
{
    if (current_token(p)->type == TOKEN_DEFINE) {
        return parse_declaration(p);
    } else if(current_token(p)->type == TOKEN_IF){
        return parse_if_statement(p);
    }
    parse_error(p, TOKEN_DEFINE, current_token(p));
    return NULL;  // unreachable
}

AstNode *parse(Parser *parser)
{
    AstNode *root = ast_create_node(AST_BLOCK);
    root->data.block = *ast_block_create();

    while (current_token(parser)->type != TOKEN_EOF) {
        ast_block_push(&root->data.block,
                       parse_statement(parser));
    }
    consume(parser, TOKEN_EOF, NULL);
    return root;
}