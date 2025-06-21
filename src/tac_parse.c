#include "tac_parse.h"
#include "tac_emit.h"
#include "tac_util.h"
#include "tac.h"
#include "ast.h"
#include <stdio.h>

// joins two TAC instruction lists
// if a is empty, returns b
// else appends b to the end of a and returns a
TACInstr *tac_concat(TACInstr *a, TACInstr *b) {
    if (!a) return b;
    TACInstr *p = a;
    while (p->next) p = p->next;
    p->next = b;
    return a;
}

/* Returns a new operand and, if needed, a list of instructions that compute it */
TACInstr *tac_get_operand(AstNode *ast, TACOperand **out, int *temp_counter) {
    if (ast->type == AST_LITERAL) {
        *out = tac_create_operand(TAC_OP_LITERAL, NULL, ast->data.literal.value);
        return NULL;
    }
    if (ast->type == AST_VARIABLE) {
        *out = tac_create_operand(TAC_OP_VAR, ast->data.variable.identifier, 0);
        return NULL;
    }
    /* otherwise it’s a sub-expression; recurse */
    TACInstr *code = tac_parse(ast, temp_counter);
    TACInstr *last = code;
    while (last && last->next) last = last->next;
    *out = last ? last->dst : NULL;
    return code;
}


/* Parse a binary expression */
TACInstr *tac_parse_binary_expression(AstNode *ast, int *temp_counter) {
    TACOperand *lhs, *rhs;
    TACInstr *code_l = tac_get_operand(ast->data.binary.left,  &lhs, temp_counter);
    TACInstr *code_r = tac_get_operand(ast->data.binary.right, &rhs, temp_counter);

    TACOperand *dst = tac_create_operand(TAC_OP_TEMP, NULL, (*temp_counter)++);
    TACInstr   *op  = tac_emit_binary_op(tac_get_binop(ast), dst, lhs, rhs);

    return tac_concat(tac_concat(code_l, code_r), op);
}

/* Parse a unary expression */
TACInstr *tac_parse_unary_expression(AstNode *ast, int *temp_counter) {
    TACOperand *src;
    TACInstr *code = tac_get_operand(ast->data.unary.operand, &src, temp_counter);

    TACOperand *dst = tac_create_operand(TAC_OP_TEMP, NULL, (*temp_counter)++);
    TACInstr   *op  = tac_emit_unary_op(tac_get_unop(ast), dst, src);

    return tac_concat(code, op);
}

TACInstr *tac_parse_literal(AstNode *ast, int *temp_counter) {
    TACOperand *dst = tac_create_operand(TAC_OP_TEMP, NULL, (*temp_counter)++);
    TACOperand *literal = tac_create_operand(TAC_OP_LITERAL, NULL, ast->data.literal.value);
    TACInstr *instr = tac_emit_copy(dst, literal);
    return instr;
}

TACInstr *tac_parse_variable(AstNode *ast, int *temp_counter) {
    TACOperand *dst = tac_create_operand(TAC_OP_TEMP, NULL, (*temp_counter)++);
    TACOperand *var = tac_create_operand(TAC_OP_VAR, ast->data.variable.identifier, 0);
    TACInstr *instr = tac_emit_copy(dst, var);
    return instr;
}

TACInstr *tac_parse_block(AstNode *ast, int *temp_counter) {
    TACInstr *code = NULL;
    for (size_t i = 0; i < ast->data.block.count; i++) {
        TACInstr *stmt_code = tac_parse(ast->data.block.statements[i], temp_counter);
        code = tac_concat(code, stmt_code);
    }
    return code;
}

TACInstr *tac_parse_if_statement(AstNode *ast, int *temp_counter) {
    // 1) Evaluate condition and emit code
    TACOperand *cond;
    TACInstr  *cond_code = tac_get_operand(ast->data.if_stmt.condition, &cond, temp_counter);

    // 2) Create label operators: else always, end only if an else-block exists
    TACOperand *label_then = tac_create_operand(TAC_OP_LABEL, NULL, (*temp_counter)++);
    TACOperand *label_end  = ast->data.if_stmt.else_block
                            ? tac_create_operand(TAC_OP_LABEL, NULL, (*temp_counter)++)
                            : NULL;

    // 3) Emit branch-on-zero to then label and label for then block
    TACInstr *branch = tac_emit_ifz(cond, label_then);
    TACInstr *label_then_code = tac_emit_label(label_then);

    // 4) Parse 'then' block
    TACInstr *then_code = tac_parse(ast->data.if_stmt.then_block, temp_counter);

    // 5) if else block exists, emit a jump over it
    //    parse the else block and emit its label
    TACInstr *jump_over_else = NULL;
    TACInstr *else_code = NULL;
    TACInstr *label_end_code = NULL;
    if (ast->data.if_stmt.else_block) {
        jump_over_else = tac_emit_goto(label_end);
        else_code = tac_parse(ast->data.if_stmt.else_block, temp_counter);
        label_end_code = tac_emit_label(label_end);
    }

 
    // 7) Build the instruction sequence
    TACInstr *seq = NULL;
    seq = tac_concat(cond_code, branch);
    seq = tac_concat(seq, then_code);
    seq = tac_concat(seq, jump_over_else);
    seq = tac_concat(seq, label_then_code);
    seq = tac_concat(seq, else_code);
    seq = tac_concat(seq, label_end_code);
    

    return seq;
}

TACInstr *tac_parse_assignment(AstNode *ast, int *temp_counter) {
    // 1) Get the LHS variable operand (no code emitted here)
    TACOperand *var;
    TACInstr    *lhs_code = tac_get_operand(ast->data.assignment.variable,
                                            &var, temp_counter);

    // 2) Compute the RHS expression (may emit code, result in 'value')
    TACOperand *value;
    TACInstr   *rhs_code = tac_get_operand(ast->data.assignment.value,
                                           &value, temp_counter);


    if(!value){
        TACInstr   *store = tac_emit_copy(var, value);
        return store;
    }

    TACInstr *tail = rhs_code;
    while (tail->next) {
        tail = tail->next;
    }
    tail->dst = var;

    return rhs_code;
}


TACInstr *tac_parse_return(AstNode *ast, int *temp_counter) {
    TACInstr *code = NULL;
    TACOperand *ret_op = NULL;
    if (ast->data.return_stmt.expression) {
        code = tac_parse(ast->data.return_stmt.expression, temp_counter);
        TACInstr *last = code; while (last->next) last = last->next;
        ret_op = last->dst;
    }
    TACInstr *ret = tac_emit_return(ret_op);
    return tac_concat(code, ret);
}

TACInstr *tac_parse_args(AstNode *ast, int *temp_counter) {
    TACInstr *code = NULL;
    for (size_t i = 0; i < ast->data.params.count; i++) {
        AstNode *param = ast->data.params.params[i];
        TACOperand *param_op = tac_create_operand(TAC_OP_VAR, param->data.variable.identifier, 0);
        TACInstr *param_instr = tac_emit_arg(param_op);
        code = tac_concat(code, param_instr);
    }
    return code;
}

TACInstr *tac_parse_function(AstNode *ast, int *temp_counter) {
    // 1) Create a label for the function
    TACOperand *label = tac_create_operand(TAC_OP_VAR, ast->data.function.name->data.variable.identifier, 0);
    TACInstr *function_instr = tac_emit_function(label);
    // 2) Parse the parameters and emit parameter instructions
    TACInstr *param_code = tac_parse_args(ast->data.function.params, temp_counter);
    // 3) Parse the function body
    TACInstr *body_code = tac_parse(ast->data.function.body, temp_counter);
    // 4) Concatenate all parts
    TACInstr *code = tac_concat(function_instr, param_code);
    code = tac_concat(code, body_code);
    // 5) Add end function instruction
    TACInstr *end_function_instr = tac_emit_end_function();
    code = tac_concat(code, end_function_instr);
    
    // 6) Return the complete function code
    return code;

}

TACInstr *tac_parse_call(AstNode *ast, int *temp_counter) {
    // 1) Evaluate arguments (which emits PARAM instrs), collecting any code
    TACInstr *args_code = NULL;
    for (size_t i = 0; ast->data.call.args && i < ast->data.call.args->data.args.count; i++) {
        AstNode *arg = ast->data.call.args->data.args.arguments[i];
        TACOperand *op;
        TACInstr   *code = tac_get_operand(arg, &op, temp_counter);
        args_code = tac_concat(args_code, code);
        args_code = tac_concat(args_code, tac_emit_param(op));
    }

    // 2) Allocate a temp for the call’s result
    TACOperand *result = tac_create_operand(TAC_OP_TEMP, NULL, (*temp_counter)++);

    // 3) Emit the call itself (it writes into ‘result’)
    TACOperand *func = tac_create_operand(
        TAC_OP_VAR,
        ast->data.call.callee->data.variable.identifier,
        0
    );
    TACInstr *call_instr = tac_emit_call(result, func, 
                          ast->data.call.args ? ast->data.call.args->data.args.count : 0);

    // 4) Concatenate and return
    return tac_concat(args_code, call_instr);
}

TACInstr *tac_parse_parameters(AstNode *ast, int *temp_counter) {
    TACInstr *code = NULL;
    for (size_t i = 0; i < ast->data.params.count; i++) {
        AstNode *param = ast->data.params.params[i];
        TACInstr *param_code = tac_parse(param, temp_counter);
        if (param_code) {
            code = tac_concat(code, param_code);
        }
        TACInstr *param_instr = tac_emit_param(param_code ? param_code->dst : NULL);
        code = tac_concat(code, param_instr);
    }
    return code;
}

TACInstr *tac_parse_while_loop(AstNode *ast, int *temp_counter) {
    // 1) Create a label for the start of the loop
    TACOperand *label_start = tac_create_operand(TAC_OP_LABEL, NULL, (*temp_counter)++);
    TACInstr *label_start_instr = tac_emit_label(label_start);

    // 2) Evaluate the condition
    TACOperand *cond;
    TACInstr *cond_code = tac_get_operand(ast->data.while_loop.condition, &cond, temp_counter);

    // 3) Create a label for the end of the loop
    TACOperand *label_end = tac_create_operand(TAC_OP_LABEL, NULL, (*temp_counter)++);
    
    // 4) Emit branch on zero to end label
    TACInstr *branch = tac_emit_ifz(cond, label_end);

    // 5) Parse the body of the loop
    TACInstr *body_code = tac_parse(ast->data.while_loop.body, temp_counter);

    // 6) Emit a jump back to the start of the loop
    TACInstr *jump_back = tac_emit_goto(label_start);

    // 7) Emit the end label
    TACInstr *label_end_instr = tac_emit_label(label_end);

    // 8) Concatenate all parts and return
    return tac_concat(tac_concat(tac_concat(tac_concat(label_start_instr, cond_code), branch),
                                 body_code), 
                      tac_concat(jump_back, label_end_instr));
}

TACInstr *tac_parse_declaration(AstNode *ast, int *temp_counter) {
    // 1) Create the variable operand for the new symbol:
    TACOperand *var = tac_create_operand(
        TAC_OP_VAR,
        ast->data.declaration.variable->data.variable.identifier,
        0
    );

    // 2) If there is no initializer, just emit a DEFINE with no value:
    if (!ast->data.declaration.value) {
        return tac_emit_define(var, NULL);
    }

    // 3) Otherwise, compute the initializer into a temp and
    //    then retarget its final dst to our new var:
    TACOperand *init_val;
    TACInstr   *init_code = tac_get_operand(
        ast->data.declaration.value,
        &init_val,
        temp_counter
    );

    // If the initializer was a bare literal/var, tac_get_operand returns NULL
    // but does give us init_val, so make sure there's at least one instruction:
    if (!init_code) {
        // emit a one-off COPY literal/var→temp
        return tac_emit_define(var, init_val);
    }

 
    TACInstr *def = tac_emit_define(var, init_val);
    return tac_concat(init_code, def);
}


/* Dispatch based on AST node */
TACInstr *tac_parse(AstNode *ast, int *temp_counter) {
    switch (ast->type) {
        case AST_BINARY_OP:
            return tac_parse_binary_expression(ast, temp_counter);
        case AST_UNARY_OP:
            return tac_parse_unary_expression(ast, temp_counter);
        case AST_LITERAL:
            return tac_parse_literal(ast, temp_counter);
        case AST_VARIABLE:
            return tac_parse_variable(ast, temp_counter);
        case AST_BLOCK: 
            return tac_parse_block(ast, temp_counter);
        case AST_IF:
            return tac_parse_if_statement(ast, temp_counter);
        case AST_ASSIGNMENT:
            return tac_parse_assignment(ast, temp_counter);
        case AST_RETURN:
            return tac_parse_return(ast, temp_counter);
        case AST_FUNCTION:
            return tac_parse_function(ast, temp_counter);
        case AST_CALL:
            return tac_parse_call(ast, temp_counter);
        case AST_PARAM_LIST:
            return tac_parse_parameters(ast, temp_counter);
        case AST_WHILE:
            return tac_parse_while_loop(ast, temp_counter);
        case AST_DECLARATION:
            return tac_parse_declaration(ast, temp_counter);

        
        /* future AST cases: ASSIGN, CALL, IF, WHILE, etc. */
        default:
            fprintf(stderr, "Unsupported AST node type %d\n", ast->type);
            return NULL;
    }
}