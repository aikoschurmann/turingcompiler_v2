#include "ast.h"
#include <stdlib.h>

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

AstNode *ast_block_create(void)
{
    AstNode *block = malloc(sizeof(*block));
    if (!block) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    block->data.block.statements = NULL;
    block->data.block.count = 0;
    block->data.block.capacity = 0; // Initialize capacity to 0
    block->type = AST_BLOCK;
    return block;
}

AstNode *ast_param_list_create(void)
{
    AstNode *params = malloc(sizeof(*params));
    if (!params) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    params->data.params.params = NULL;
    params->data.params.count = 0;
    params->data.params.capacity = 0; // Initialize capacity to 0
    params->type = AST_PARAM_LIST;
    return params;
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

void ast_param_list_push(AstNode *param_list, AstNode *param) {
    if (param_list->data.params.count == param_list->data.params.capacity) {
        size_t newcap = param_list->data.params.capacity ? param_list->data.params.capacity * 2 : 4;
        param_list->data.params.params = realloc(
            param_list->data.params.params,
            newcap * sizeof(AstNode*)
        );
        if (!param_list->data.params.params) {
            perror("realloc");
            exit(EXIT_FAILURE);
        }
        param_list->data.params.capacity = newcap;
    }
    param_list->data.params.params[param_list->data.params.count++] = param;
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
        for (size_t i = 0; i < fn->params->data.params.count; i++) {
            free(fn->params->data.params.params[i]->data.variable.identifier);
            free(fn->params->data.params.params[i]);
        }
        free(fn->params->data.params.params);
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
        free(call->callee->data.variable.identifier);
        free(call->callee);
        for (size_t i = 0; i < call->args->data.args.count; i++) {
            free_ast_node(call->args->data.args.arguments[i]);
        }
        break;
    }

    default:
        break;
    }
    free(node);
}