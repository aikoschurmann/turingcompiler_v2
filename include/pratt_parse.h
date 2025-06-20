#pragma once

#include "ast.h"
#include "parser.h"

AstNode *parse_expression_pratt(Parser *p, int min_bp);
AstNode *parse_prefix(Parser *p);
AstNode *parse_infix(Parser *p, AstNode *lhs, int min_bp);

int prefix_binding_power(const char *op);
void infix_binding_power(const char *op, int *l_bp, int *r_bp);
int is_prefix_op(const char *op);