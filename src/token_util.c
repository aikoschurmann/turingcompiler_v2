#include "token_util.h"

char *astnode_type_to_string(AstNodeType type)
{
    switch (type) {
        case AST_LITERAL:      return "AST_LITERAL";
        case AST_VARIABLE:     return "AST_VARIABLE";
        case AST_UNARY_OP:     return "AST_UNARY_OP";
        case AST_BINARY_OP:    return "AST_BINARY_OP";
        case AST_IF:           return "AST_IF";
        case AST_WHILE:        return "AST_WHILE";
        case AST_BLOCK:        return "AST_BLOCK";
        case AST_FUNCTION:     return "AST_FUNCTION";
        case AST_DECLARATION:  return "AST_DECLARATION";
        case AST_ASSIGNMENT:   return "AST_ASSIGNMENT";
        case AST_RETURN:       return "AST_RETURN";
        case AST_CALL:         return "AST_CALL";
        default:               return "UNKNOWN_AST_NODE";
    }
}

char *binaryop_to_string(BinaryOp op)
{
    switch (op) {
        case OP_ADD: return "+";
        case OP_SUB: return "-";
        case OP_MUL: return "*";
        case OP_DIV: return "/";
        case OP_LT:  return "<";
        case OP_EQ:  return "==";
        case OP_GT:  return ">";
        case OP_LEQ: return "<=";
        case OP_GEQ: return ">=";
        case OP_NEQ: return "!=";
        default:     return "UNKNOWN_BINARY_OP";
    }
}

char *unarop_to_string(UnaryOp op)
{
    switch (op) {
        case UN_OP_NEG: return "-";
        case UN_OP_NOT: return "!";
        default:        return "UNKNOWN_UNARY_OP";
    }
}