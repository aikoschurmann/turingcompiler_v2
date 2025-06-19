#pragma once
#include "token.h"



typedef struct {
    TokenArray tokens;
    size_t       start;     // new: index of first token in this sub‑parser
    size_t       end;       // new: one past the last token to parse
    size_t       current;   // always lives in [start..end]
    char        *filename;
} Parser;




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
typedef struct { char *identifier; int test } AstVariable;
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
    AstVariable **params;
    size_t        count;
    size_t        capacity;
} AstParamList;

typedef struct {
    AstNode   **arguments;
    size_t      count;
    size_t      capacity;
} AstArgList;

typedef struct {
    AstVariable *name; // Function name
    AstParamList *params; // Function parameters
    AstBlock *body; // Function body
} AstFunction;

typedef struct {
    AstVariable *callee; // Function being called
    AstArgList *args; // Arguments to the function call
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
    } data;
};

void free_ast_node(AstNode *node);
Parser *parser_create(TokenArray tokens, const char *filename);

AstNode *ast_create_node(AstNodeType type);
AstNode *parse(Parser *parser);
void parser_free(Parser *parser);
const char *astnode_type_to_string(AstNodeType type);
void ast_block_push(AstBlock *b, AstNode *stmt);
char *binaryop_to_string(BinaryOp op);
void print_ast(AstNode *node, int indent);

void infix_binding_power(const char *op, int *l_bp, int *r_bp);
int prefix_binding_power(const char *op);
int is_prefix_op(const char *op);
void parse_error(const Parser *parser, TokenType expected, const Token *actual);
AstBlock *ast_block_create(void);
Token *consume(Parser *p, TokenType expected, const char *value);
Token *current_token(Parser *p);
Token *peek(Parser *p, size_t offset);