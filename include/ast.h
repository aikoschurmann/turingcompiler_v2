#pragma once

#include <stddef.h>
#include <stdio.h>
#include <string.h>


typedef struct AstNode AstNode;

typedef enum {
    AST_LITERAL,
    AST_VARIABLE,
    AST_UNARY_OP,
    AST_BINARY_OP,
    AST_IF,
    AST_WHILE,
    AST_BLOCK,
    AST_FUNCTION, // Function definition or declaration
    AST_DECLARATION, // Variable declaration
    AST_ASSIGNMENT, // Variable assignment
    AST_RETURN, // Return statement
    AST_CALL, // Function call
    AST_ARG_LIST, // List of arguments for function calls
    AST_PARAM_LIST, // List of parameters for function definitions
} AstNodeType;

// Binary operation
typedef enum {
    OP_ADD,
    OP_MUL,
    OP_DIV,
    OP_SUB,
    OP_LT,
    OP_EQ,
    OP_GT,
    OP_LEQ,
    OP_GEQ,
    OP_NEQ,
} BinaryOp;

typedef enum {
    UN_OP_NEG, // Unary negation
    UN_OP_NOT, // Logical NOT
} UnaryOp;


typedef struct {
    AstNode **statements;
    size_t    count;
    size_t    capacity;
} AstBlock;

typedef struct { int value; }      AstLiteral;
typedef struct { char *identifier;} AstVariable;
typedef struct { AstNode *operand; UnaryOp op; }   AstUnaryOp;
typedef struct { AstNode *left, *right; BinaryOp op; } AstBinaryOp;
typedef struct { AstNode *condition; AstBlock *then_block, *else_block; } AstIfStatement;
typedef struct { AstNode *condition; AstBlock *body; } AstWhileLoop;

typedef struct {
    AstNode *variable; // Variable being declared
    AstNode *value;
} AstDeclaration;

typedef struct {
    AstNode *variable; // Variable being assigned
    AstNode *value; // Value being assigned
} AstAssignment;

typedef struct {
    AstNode *expression; // Expression to return
} AstReturn;

typedef struct {
    AstNode       **params;
    size_t        count;
    size_t        capacity;
} AstParamList;

typedef struct {
    AstNode     **arguments;
    size_t      count;
    size_t      capacity;
} AstArgList;

typedef struct {
    AstNode *name; // Function name
    AstNode *params; // Function parameters
    AstBlock *body; // Function body
} AstFunction;

typedef struct {
    AstNode *callee; // Function being called
    AstNode *args; // Arguments to the function call
} AstCall;



struct AstNode {
    AstNodeType type;
    union {
        AstLiteral      literal;
        AstVariable     variable;
        AstUnaryOp      unary;
        AstBinaryOp     binary;
        AstIfStatement  if_stmt;
        AstWhileLoop    while_loop;
        AstBlock        block;
        AstFunction     function;
        AstDeclaration  declaration;
        AstAssignment   assignment;
        AstReturn       return_stmt;
        AstCall         call;
        AstArgList      args; // For function calls
        AstParamList    params; // For function definitions
    } data;
};

AstNode *ast_create_node(AstNodeType type);

AstNode *ast_block_create(void);

AstNode *ast_param_list_create(void);

void ast_block_push(AstBlock *block, AstNode *statement);

void ast_param_list_push(AstNode *param_list, AstNode *param);

void free_ast_node(AstNode *node);