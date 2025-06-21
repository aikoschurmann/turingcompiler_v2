#pragma once
#include "tac.h"


TACOperand *tac_create_operand(TACOperandType type, const char *name, int literal);
// t = a + b, t = a * b, etc.
TACInstr *tac_emit_binary_op(TACBinOp binop, TACOperand *dst, TACOperand *arg1, TACOperand *arg2);
// t = -a
TACInstr *tac_emit_unary_op(TACUnaryOp unop, TACOperand *dst, TACOperand *arg1);
// t = a
TACInstr *tac_emit_copy(TACOperand *dst, TACOperand *arg1);
// label:
TACInstr *tac_emit_label(TACOperand *dst);
// goto label
TACInstr *tac_emit_goto(TACOperand *arg1);
// t0 = a < b
// ifz t0 goto label
TACInstr *tac_emit_ifz(TACOperand *arg1, TACOperand *arg2);
// param x
TACInstr *tac_emit_param(TACOperand *arg1);
// t = call f, n_args
TACInstr *tac_emit_call(TACOperand *dst, TACOperand *arg1, int n_args);
// return t or return
TACInstr *tac_emit_return(TACOperand *arg1);
// fun name
TACInstr *tac_emit_function(TACOperand *dst);
// End of function definition
TACInstr *tac_emit_end_function(void);

void tac_free_operand(TACOperand *operand);

void tac_free_instr(TACInstr *instr);