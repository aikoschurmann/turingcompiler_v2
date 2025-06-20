#pragma once
#include "ast.h"


void print_json_fp(FILE *out, AstNode *n);
void dump_ast_json_file(const char *filename, AstNode *root);
void print_ast(AstNode *node, int indent);