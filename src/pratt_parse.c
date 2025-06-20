#include "pratt_parse.h"
#include "token_util.h"
#include "parse_statements.h"
#include "parse_error.h"

BinaryOp get_binary_operator(const char *op) {
    if      (strcmp(op, "+")  == 0) return OP_ADD;
    else if (strcmp(op, "-")  == 0) return OP_SUB;
    else if (strcmp(op, "*")  == 0) return OP_MUL;
    else if (strcmp(op, "/")  == 0) return OP_DIV;
    else if (strcmp(op, "<")  == 0) return OP_LT;
    else if (strcmp(op, ">")  == 0) return OP_GT;
    else if (strcmp(op, "<=") == 0) return OP_LEQ;
    else if (strcmp(op, ">=") == 0) return OP_GEQ;
    else if (strcmp(op, "==") == 0) return OP_EQ;
    else if (strcmp(op, "!=") == 0) return OP_NEQ;
    else parse_error(NULL, TOKEN_OPERATOR, NULL);
}

// Entry point for Pratt parsing
AstNode *parse_expression_pratt(Parser *p, int min_bp) {
    AstNode *lhs = parse_prefix(p);
    return parse_infix(p, lhs, min_bp);
}

// Parse prefix (atomic, variable, literal, unary, or grouped)
AstNode *parse_prefix(Parser *p) {
    Token *tok = current_token(p);

    switch (tok->type) {
        case TOKEN_NUMBER: {
            AstNode *node = ast_create_node(AST_LITERAL);
            node->data.literal.value = atoi(tok->value);
            consume(p, TOKEN_NUMBER, NULL);
            return node;
        }

        case TOKEN_IDENTIFIER: {
            if (peek(p, 1) && peek(p, 1)->type == TOKEN_PAREN_OPEN) {
                return parse_function_call(p);
            } else {
                AstNode *node = ast_create_node(AST_VARIABLE);
                node->data.variable.identifier = strdup(tok->value);
                consume(p, TOKEN_IDENTIFIER, NULL);
                return node;
            }
        }

        case TOKEN_OPERATOR: {
            if (!is_prefix_op(tok->value)) break;

            const char *op = tok->value;
            int r_bp = prefix_binding_power(op);
            consume(p, TOKEN_OPERATOR, NULL);

            AstNode *operand = parse_expression_pratt(p, r_bp);
            AstNode *node = ast_create_node(AST_UNARY_OP);
            node->data.unary.op = op[0]; // Assuming single-char prefix ops
            node->data.unary.operand = operand;
            return node;
        }

        case TOKEN_PAREN_OPEN: {
            consume(p, TOKEN_PAREN_OPEN, NULL);
            AstNode *node = parse_expression_pratt(p, 0);
            consume(p, TOKEN_PAREN_CLOSE, NULL);
            return node;
        }

        default:
            parse_error(p, TOKEN_NUMBER, tok);
            return NULL;
    }
}

// Parse infix expressions (left-associative loop)
AstNode *parse_infix(Parser *p, AstNode *lhs, int min_bp) {
    while (1) {
        Token *tok = peek(p, 0);
        if (!tok || tok->type != TOKEN_OPERATOR) break;

        const char *op = tok->value;
        if (strcmp(op, ")") == 0) break;

        int l_bp, r_bp;
        infix_binding_power(op, &l_bp, &r_bp);
        if (l_bp < min_bp) break;

        consume(p, TOKEN_OPERATOR, NULL);
        AstNode *rhs = parse_expression_pratt(p, r_bp);

        AstNode *bin = ast_create_node(AST_BINARY_OP);
        bin->data.binary.op = get_binary_operator(op);
        bin->data.binary.left = lhs;
        bin->data.binary.right = rhs;

        lhs = bin;
    }
    return lhs;
}

// Define which operators are prefix
int is_prefix_op(const char *op) {
    return  strcmp(op, "-") == 0 || strcmp(op, "!") == 0;
}

// Prefix binding power (right-binding)
int prefix_binding_power(const char *op) {
    if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0) return 5;
    if (strcmp(op, "!") == 0)                        return 6;
    return 0;
}

// Infix binding powers
void infix_binding_power(const char *op, int *l_bp, int *r_bp) {
    // Highest: multiplicative
    if (strcmp(op, "*") == 0 || strcmp(op, "/") == 0) {
        *l_bp = 7; *r_bp = 8;
    }
    // Next: additive
    else if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0) {
        *l_bp = 5; *r_bp = 6;
    }
    // Comparisons
    else if (!strcmp(op, "<") || !strcmp(op, ">")
          || !strcmp(op, "<=") || !strcmp(op, ">=")
          || !strcmp(op, "==")|| !strcmp(op, "!=")) {
        *l_bp = 3; *r_bp = 4;
    }
    // Assignment (right-associative, lowest)
    else if (strcmp(op, "=") == 0) {
        *l_bp = 1; *r_bp = 2;
    }
    else {
        *l_bp = 0; *r_bp = 0;
    }
}