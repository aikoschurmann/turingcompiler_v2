#pragma once
#include "tac.h"
#include "ast.h"

TACInstr *tac_parse_expression(AstNode *ast, int *temp_counter);
TACInstr *tac_parse(AstNode *ast, int *temp_counter);
void tac_print_list(TACInstr *head);