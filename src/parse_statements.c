#include "parse_statements.h"
#include "parse_error.h"
#include "pratt_parse.h"

size_t parser_find_first_token(Parser *p, TokenType type)
{   
    for (size_t i = p->current; i < p->end; i++) {
        if (p->tokens.data[i]->type == type) {
            return i;
        }
    }

    ParseError *err = create_parse_error(
        p->tokens.data[p->end - 1]->line,
        p->tokens.data[p->end - 1]->column,
        p->filename,
        "Expected token not found",
        token_type_to_string(type),
        p->tokens.data[p->end - 1]->value,
        1 // is_fatal
    );
    report_parse_error(err);
}

size_t parser_find_matching(Parser *p, TokenType open, TokenType close) {
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
    ParseError *err = create_parse_error(
        p->tokens.data[p->current - 1]->line,
        p->tokens.data[p->current - 1]->column,
        p->filename,
        "unmatched close token",
        token_type_to_string(close),
        NULL,
        1 // is_fatal
    );
    report_parse_error(err);
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

AstNode *parse_while_loop(Parser *p)
{
    consume(p, TOKEN_WHILE, NULL);
    consume(p, TOKEN_PAREN_OPEN, NULL);

    AstNode *condition = parse_expression(p);

    consume(p, TOKEN_PAREN_CLOSE, NULL);
    consume(p, TOKEN_BRACE_OPEN, NULL);

    // Parse the loop body
    size_t body_end = parser_find_matching(p, TOKEN_BRACE_OPEN, TOKEN_BRACE_CLOSE);
    Parser body_parser = parser_slice(p, p->current, body_end);
    
    AstNode *body_block = parse(&body_parser);
    p->current = body_end;
    consume(p, TOKEN_BRACE_CLOSE, NULL);

    // Create the 'while' node
    AstNode *while_node = ast_create_node(AST_WHILE);
    while_node->data.while_loop.condition = condition;
    while_node->data.while_loop.body = (AstBlock *)body_block;

    AstDeclaration *decl = ast_create_node(AST_DECLARATION);

    return while_node;
}



AstNode *parse_assignment(Parser *p) {
    // Assignment: identifier = expression
    Token *var = consume(p, TOKEN_IDENTIFIER, NULL);
    consume(p, TOKEN_OPERATOR, "=");
    AstNode *value = parse_expression(p);
    consume(p, TOKEN_END_OF_LINE, NULL);

    AstNode *assignment = ast_create_node(AST_ASSIGNMENT);
    AstNode *variable = ast_create_node(AST_VARIABLE);
    variable->data.variable.identifier = strdup(var->value);

    assignment->data.assignment.variable = variable;
    assignment->data.assignment.value = value;

    return assignment;
}

AstNode *parse_arg_list(Parser *p) {
    AstNode *args_node = ast_param_list_create();

    consume(p, TOKEN_PAREN_OPEN, NULL);

    while (current_token(p)->type != TOKEN_PAREN_CLOSE) {
        AstNode *arg = parse_expression(p);
        ast_param_list_push(args_node, arg);

        if (current_token(p)->type == TOKEN_COMMA) {
            consume(p, TOKEN_COMMA, NULL);
        }
    }

    consume(p, TOKEN_PAREN_CLOSE, NULL);
    return args_node;
}

AstNode *parse_function_call(Parser *p) {
    // Function call: identifier ( arguments )
    Token *fn_name = consume(p, TOKEN_IDENTIFIER, NULL);
    AstNode *call_node = ast_create_node(AST_CALL);
    call_node->data.call.callee = ast_create_node(AST_VARIABLE);
    call_node->data.call.callee->data.variable.identifier = strdup(fn_name->value);

    AstNode *args_node = parse_arg_list(p);

    call_node->data.call.args = args_node;

    return call_node;
}



AstNode *parse_identifier(Parser *p) {
    Token *next = peek(p, 1);

    if (next && next->type == TOKEN_OPERATOR && strcmp(next->value, "=") == 0) {
       return parse_assignment(p);
    }

    if (next && next->type == TOKEN_PAREN_OPEN) {
       AstNode *res = parse_expression(p);
       consume(p, TOKEN_END_OF_LINE, NULL);
       return res;
    }

    AstNode *expr = parse_expression(p);
    consume(p, TOKEN_END_OF_LINE, NULL);
    return expr;
}

AstNode *parse_operator(Parser *p)
{
    Token *op = current_token(p);
    if(is_prefix_op(op->value)) {
        AstNode *exp = parse_expression(p);
        consume(p, TOKEN_END_OF_LINE, NULL);
        return exp;

    } else {
        ParseError *err = create_parse_error(op->line, op->column, p->filename,
                           "Unexpected operator",
                           "a prefix operator", op->value, 1);
        report_parse_error(err);
    }
}

AstNode *parse_return_statement(Parser *p)
{
    consume(p, TOKEN_RETURN, NULL);
    AstNode *return_node = ast_create_node(AST_RETURN);

    if (current_token(p)->type != TOKEN_END_OF_LINE) {
        return_node->data.return_stmt.expression = parse_expression(p);
    } else {
        return_node->data.return_stmt.expression = NULL;
    }

    consume(p, TOKEN_END_OF_LINE, NULL);
    return return_node;
}

AstNode *parse_number(Parser *p)
{   Token *next = peek(p, 1);
    if (next && next->type == TOKEN_OPERATOR && next->value[0] == '=') {
        parse_error(p, TOKEN_OPERATOR, next);
    }
    AstNode *exp = parse_expression_pratt(p, 0);
    consume(p, TOKEN_END_OF_LINE, NULL);
    return exp;
}

AstNode *parse_block(Parser *p)
{
    consume(p, TOKEN_BRACE_OPEN, NULL);
    
    size_t block_end = parser_find_matching(p, TOKEN_BRACE_OPEN, TOKEN_BRACE_CLOSE);
    Parser block_parser = parser_slice(p, p->current, block_end);
    AstNode *block = parse(&block_parser);
    p->current = block_end;
    consume(p, TOKEN_BRACE_CLOSE, NULL);

    return block;

}

AstNode *parse_parameters(Parser *p) {
    AstNode *params_node = ast_param_list_create();

    consume(p, TOKEN_PAREN_OPEN, NULL);

    while (current_token(p)->type != TOKEN_PAREN_CLOSE) {
        Token *param_name = consume(p, TOKEN_IDENTIFIER, NULL);
        AstNode *param_node = ast_create_node(AST_VARIABLE);
        param_node->data.variable.identifier = strdup(param_name->value);

        ast_param_list_push(params_node, param_node);

        if (current_token(p)->type == TOKEN_COMMA) {
            consume(p, TOKEN_COMMA, NULL);
        }
    }

    consume(p, TOKEN_PAREN_CLOSE, NULL);
    return params_node;
}


AstNode* parse_function_definition(Parser *p)
{
    consume(p, TOKEN_FUNCTION, NULL);
    Token *name = consume(p, TOKEN_IDENTIFIER, NULL);

    AstNode *fn_node = ast_create_node(AST_FUNCTION);
    AstNode *name_node = ast_create_node(AST_VARIABLE);
    name_node->data.variable.identifier = strdup(name->value);
    fn_node->data.function.name = name_node;
    fn_node->data.function.params = NULL;

    AstNode *params = parse_parameters(p);

    fn_node->data.function.params = params;
    consume(p, TOKEN_BRACE_OPEN, NULL);
    size_t body_end = parser_find_matching(p, TOKEN_BRACE_OPEN, TOKEN_BRACE_CLOSE);
    Parser body_parser = parser_slice(p, p->current, body_end);
    AstNode *body_block = parse(&body_parser);
    p->current = body_end;
    consume(p, TOKEN_BRACE_CLOSE, NULL);
    fn_node->data.function.body = (AstBlock *)body_block;

    return fn_node;
}

AstNode *parse_statement(Parser *p)
{
    
    switch (current_token(p)->type)
    {
        case TOKEN_DEFINE:
            return parse_declaration(p);
        case TOKEN_IF:
            return parse_if_statement(p);
        case TOKEN_WHILE:
            return parse_while_loop(p);
        case TOKEN_RETURN: 
            return parse_return_statement(p);
        case TOKEN_IDENTIFIER:
            return parse_identifier(p);
        case TOKEN_NUMBER:
            return parse_number(p);
        case TOKEN_OPERATOR:
            return parse_operator(p);
        case TOKEN_PAREN_OPEN:
            return parse_number(p); 
        case TOKEN_PAREN_CLOSE:
            // This should not happen in a well-formed program
            parse_error(p, TOKEN_PAREN_OPEN, current_token(p));
            return NULL;  // unreachable
        case TOKEN_BRACE_OPEN:
            return parse_block(p);
        case TOKEN_BRACE_CLOSE:
            // This should not happen in a well-formed program
            parse_error(p, TOKEN_BRACE_OPEN, current_token(p));
            return NULL;  // unreachable
        case TOKEN_END_OF_LINE:
            // Just skip empty lines
            consume(p, TOKEN_END_OF_LINE, NULL);
            return NULL;  // no AST node for empty lines
        case TOKEN_EOF:
            // End of file, return NULL to indicate no more statements
            return NULL;
        case TOKEN_FUNCTION:
            return parse_function_definition(p);
                    
        default:
            parse_error(p, TOKEN_DEFINE, current_token(p));
            return NULL;  // unreachable
    }

}

AstNode *parse(Parser *parser)
{
    AstNode *root = ast_create_node(AST_BLOCK);
    

    while (current_token(parser)->type != TOKEN_EOF) {
        ast_block_push(&root->data.block,
                       parse_statement(parser));
    }
    consume(parser, TOKEN_EOF, NULL);
    return root;
}