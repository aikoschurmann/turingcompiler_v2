#include "pratt_parse.h"

// Pratt parser core
AstNode *parse_expression_pratt(Parser *p, int min_bp) {
    Token *tok = current_token(p);
    AstNode *lhs = NULL;

    // Prefix (or atomic) resolution
    if (tok->type == TOKEN_NUMBER) {
        lhs = ast_create_node(AST_LITERAL);
        lhs->data.literal.value = atoi(tok->value);
        consume(p, TOKEN_NUMBER, NULL);
    } else if (tok->type == TOKEN_IDENTIFIER) {
        lhs = ast_create_node(AST_VARIABLE);
        lhs->data.variable.identifier = strdup(tok->value);
        consume(p, TOKEN_IDENTIFIER, NULL);
    } else if (tok->type == TOKEN_OPERATOR && is_prefix_op(tok->value)) {
        const char *op = tok->value;
        consume(p, TOKEN_OPERATOR, NULL);
        int r_bp = prefix_binding_power(op);
        AstNode *operand = parse_expression_pratt(p, r_bp);
        lhs = ast_create_node(AST_UNARY_OP);
        lhs->data.unary.op = op[0];
        lhs->data.unary.operand = operand;
    } else if (tok->type == TOKEN_PAREN_OPEN) {
        consume(p, TOKEN_PAREN_OPEN, NULL);
        lhs = parse_expression_pratt(p, 0);
        consume(p, TOKEN_PAREN_CLOSE, NULL);
    } else {
        parse_error(p, TOKEN_NUMBER, tok);
    }

    // Infix loop
    while (1) {
        Token *peeked = peek(p, 0);
        if (!peeked || peeked->type != TOKEN_OPERATOR) break;
        const char *op = peeked->value;
        if (strcmp(op, ")") == 0) break;

        int l_bp, r_bp;
        infix_binding_power(op, &l_bp, &r_bp);
        if (l_bp < min_bp) break;

        // consume operator
        consume(p, TOKEN_OPERATOR, NULL);
        // parse right-hand side
        AstNode *rhs = parse_expression_pratt(p, r_bp);

        // build binary node
        AstNode *bin = ast_create_node(AST_BINARY_OP);
        // map string op to enum
        if        (strcmp(op, "+")  == 0) bin->data.binary.op = OP_ADD;
        else if   (strcmp(op, "-")  == 0) bin->data.binary.op = OP_SUB;
        else if   (strcmp(op, "*")  == 0) bin->data.binary.op = OP_MUL;
        else if   (strcmp(op, "/")  == 0) bin->data.binary.op = OP_DIV;
        else if   (strcmp(op, "<")  == 0) bin->data.binary.op = OP_LT;
        else if   (strcmp(op, ">")  == 0) bin->data.binary.op = OP_GT;
        else if   (strcmp(op, "<=") == 0) bin->data.binary.op = OP_LEQ;
        else if   (strcmp(op, ">=") == 0) bin->data.binary.op = OP_GEQ;
        else if   (strcmp(op, "==") == 0) bin->data.binary.op = OP_EQ;
        else if   (strcmp(op, "!=") == 0) bin->data.binary.op = OP_NEQ;
        else {
            parse_error(p, TOKEN_OPERATOR, peeked);
        }
        bin->data.binary.left  = lhs;
        bin->data.binary.right = rhs;
        lhs = bin;
    }

    return lhs;
}

// Define which operators are prefix
int is_prefix_op(const char *op) {
    return strcmp(op, "+") == 0 || strcmp(op, "-") == 0 || strcmp(op, "!") == 0;
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