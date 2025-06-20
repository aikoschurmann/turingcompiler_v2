#pragma once
#include "ast.h"
#include "parser.h"

size_t parser_find_first_token(Parser *p, TokenType type);

size_t parser_find_matching(Parser *p, TokenType open, TokenType close);

AstNode *parse_expression(Parser *p);

AstNode *parse_declaration(Parser *p);

AstNode *parse_if_statement(Parser *p);

AstNode *parse_while_loop(Parser *p);

AstNode *parse_assignment(Parser *p);

AstNode *parse_arg_list(Parser *p);

AstNode *parse_function_call(Parser *p);

AstNode *parse_identifier(Parser *p);

AstNode *parse_operator(Parser *p);

AstNode *parse_return_statement(Parser *p);

AstNode *parse_number(Parser *p);

AstNode *parse_block(Parser *p);

AstNode *parse_parameters(Parser *p);

AstNode *parse_function_definition(Parser *p);

AstNode *parse_statement(Parser *p);

AstNode *parse(Parser *parser);