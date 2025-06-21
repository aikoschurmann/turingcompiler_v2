#include "tac_util.h"

/* Map AST binary ops to TAC ops */
TACBinOp tac_get_binop(AstNode *ast) {
    switch (ast->data.binary.op) {
        case OP_ADD: return TAC_ADD;
        case OP_SUB: return TAC_SUB;
        case OP_MUL: return TAC_MUL;
        case OP_DIV: return TAC_DIV;
        case OP_LT:  return TAC_LT;
        case OP_LEQ: return TAC_LTE;
        case OP_GT:  return TAC_GT;
        case OP_GEQ: return TAC_GTE;
        case OP_EQ:  return TAC_EQ;
        case OP_NEQ: return TAC_NEQ;
        default:
            fprintf(stderr, "Unsupported BINARY op %d\n", ast->data.binary.op);
            return TAC_ADD;
    }
}

/* Map AST unary ops to TAC unary ops */
TACUnaryOp tac_get_unop(AstNode *ast) {
    switch (ast->data.unary.op) {
        case UN_OP_NEG: return TAC_NEG;
        case UN_OP_NOT: return TAC_NOT;
        default:
            fprintf(stderr, "Unsupported UNARY op %d\n", ast->data.unary.op);
            return TAC_NEG;
    }
}

/* Human-readable op strings */
const char *tac_binop_str(TACBinOp o) {
    switch(o) {
      case TAC_ADD:  return "+";  case TAC_SUB:  return "-";
      case TAC_MUL:  return "*";  case TAC_DIV:  return "/";
      case TAC_EQ:   return "=="; case TAC_NEQ: return "!=";
      case TAC_LT:   return "<";  case TAC_LTE: return "<=";
      case TAC_GT:   return ">";  case TAC_GTE: return ">=";
      case TAC_AND:  return "&&"; case TAC_OR:  return "||";
      default:       return "?";
    }
}
const char *tac_unop_str(TACUnaryOp o) {
    switch(o) {
      case TAC_NEG: return "-";
      case TAC_NOT: return "!";
      default:      return "?";
    }
}