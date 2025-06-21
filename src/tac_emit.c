#include "tac_emit.h"
#include <stdlib.h> 
#include <string.h>
#include <stdio.h>

TACOperand *tac_create_operand(TACOperandType type, const char *name, int literal) {
    TACOperand *operand = malloc(sizeof(TACOperand));
    if (!operand) return NULL; // Handle memory allocation failure
    operand->type = type;
    
    if (type == TAC_OP_VAR) {
        operand->name = strdup(name); // Duplicate the string for safety
        if (!operand->name) {
            free(operand);
            return NULL; // Handle memory allocation failure
        }
    } 
    
    if( type == TAC_OP_LITERAL || type == TAC_OP_TEMP  || type == TAC_OP_LABEL) {
        operand->literal = literal; // For LITERAL type
    }
    return operand;
}

TACInstr *tac_emit_binary_op(TACBinOp binop, TACOperand *dst, TACOperand *arg1, TACOperand *arg2) {
    TACInstr *instr = malloc(sizeof(TACInstr));
    if (!instr) return NULL; // Handle memory allocation failure
    instr->kind = TAC_BINARY_OP;
    instr->op.binop = binop;
    instr->dst = dst;
    instr->arg1 = arg1;
    instr->arg2 = arg2;
    return instr;
}

TACInstr *tac_emit_unary_op(TACUnaryOp unop, TACOperand *dst, TACOperand *arg1) {
    TACInstr *instr = malloc(sizeof(TACInstr));
    if (!instr) return NULL; // Handle memory allocation failure
    instr->kind = TAC_UNARY_OP;
    instr->op.unop = unop;
    instr->dst = dst;
    instr->arg1 = arg1;
    instr->arg2 = NULL; // Unary operations do not have a second argument
    return instr;
}

TACInstr *tac_emit_copy(TACOperand *dst, TACOperand *arg1) {
    TACInstr *instr = malloc(sizeof(TACInstr));
    if (!instr) return NULL; // Handle memory allocation failure
    instr->kind = TAC_COPY;
    instr->dst = dst;
    instr->arg1 = arg1;
    instr->arg2 = NULL; // Copy does not have a second argument
    return instr;
}

TACInstr *tac_emit_label(TACOperand *dst) {
    TACInstr *instr = malloc(sizeof(TACInstr));
    if (!instr) return NULL; // Handle memory allocation failure
    instr->kind = TAC_LABEL;
    instr->dst = dst;
    instr->arg1 = NULL; // Labels do not have arguments
    instr->arg2 = NULL; // Labels do not have arguments
    return instr;
}

TACInstr *tac_emit_goto(TACOperand *arg1) {
    TACInstr *instr = malloc(sizeof(TACInstr));
    if (!instr) return NULL; // Handle memory allocation failure
    instr->kind = TAC_GOTO;
    instr->dst = NULL; // Goto does not have a destination
    instr->arg1 = arg1;
    instr->arg2 = NULL; // Goto does not have a second argument
    return instr;
}

TACInstr *tac_emit_ifz(TACOperand *arg1, TACOperand *arg2) {
    TACInstr *instr = malloc(sizeof(TACInstr));
    if (!instr) return NULL; // Handle memory allocation failure
    instr->kind = TAC_IFZ;
    instr->dst = NULL; // Ifz does not have a destination
    instr->arg1 = arg1; // The first argument is the operand to check
    instr->arg2 = arg2; // The second argument is the label to jump to
    return instr;
}

TACInstr *tac_emit_param(TACOperand *arg1) {
    TACInstr *instr = malloc(sizeof(TACInstr));
    if (!instr) return NULL; // Handle memory allocation failure
    instr->kind = TAC_PARAM;
    instr->dst = NULL; // Param does not have a destination
    instr->arg1 = arg1; // The first argument is the parameter to pass
    instr->arg2 = NULL; // Param does not have a second argument
    return instr;
}

TACInstr *tac_emit_call(TACOperand *dst, 
                        TACOperand *arg1, 
                        int n_args) 
{
    TACInstr *instr = malloc(sizeof(TACInstr));
    if (!instr) return NULL; // Handle memory allocation failure

    instr->kind = TAC_CALL;
    instr->dst = dst; // The destination for the result of the call 
    instr->arg1 = arg1; // The function to call
    // Create an operand for the number of arguments
    instr->arg2 = tac_create_operand(TAC_OP_LITERAL, NULL, n_args); 
    return instr;
}

TACInstr *tac_emit_return(TACOperand *arg1) {
    TACInstr *instr = malloc(sizeof(TACInstr));
    if (!instr) return NULL; // Handle memory allocation failure
    instr->kind = TAC_RETURN;
    instr->dst = NULL; // Return does not have a destination
    instr->arg1 = arg1; // The operand to return, can be NULL for void return
    instr->arg2 = NULL; // Return does not have a second argument
    return instr;
}

TACInstr *tac_emit_function(TACOperand *dst) {
    TACInstr *instr = malloc(sizeof(TACInstr));
    if (!instr) return NULL; // Handle memory allocation failure
    instr->kind = TAC_FUNCTION;
    instr->dst = dst; // The function name as a label
    instr->arg1 = NULL; // Function does not have an argument
    instr->arg2 = NULL; // Function does not have a second argument
    return instr;
}

TACInstr *tac_emit_end_function(void) {
    TACInstr *instr = malloc(sizeof(TACInstr));
    if (!instr) return NULL; // Handle memory allocation failure
    instr->kind = TAC_END_FUNCTION;
    instr->dst = NULL; // End function does not have a destination
    instr->arg1 = NULL; // End function does not have an argument
    instr->arg2 = NULL; // End function does not have a second argument
    return instr;
}

void tac_free_operand(TACOperand *operand) {
    if (operand) {
        if (operand->name) {
            free(operand->name); // Free the name if it was allocated
        }
        free(operand); // Free the operand structure itself
    }
}

void tac_free_instr(TACInstr *instr) {
    if (instr) {
        tac_free_operand(instr->dst);
        tac_free_operand(instr->arg1);
        tac_free_operand(instr->arg2);
        free(instr); // Free the instruction structure itself
    }
}