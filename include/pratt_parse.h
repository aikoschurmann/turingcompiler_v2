#pragma once

#include "parser.h"

AstNode *parse_expression_pratt(Parser *p, int min_bp);