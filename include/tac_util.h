#pragma once

#include "tac.h"
#include "ast.h"

TACBinOp tac_get_binop(AstNode *ast);
TACUnaryOp tac_get_unop(AstNode *ast);
const char *tac_binop_str(TACBinOp o);
const char *tac_unop_str(TACUnaryOp o);

