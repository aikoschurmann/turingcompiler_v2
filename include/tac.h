#pragma once

typedef enum {
    TAC_OP_TEMP,
    TAC_OP_VAR,
    TAC_OP_LITERAL,
    TAC_OP_LABEL
} TACOperandType;

typedef struct {
    TACOperandType type;
    union {
        char *name;        // for TEMP/VAR/LABEL
        int literal;      // for LITERAL
    };
} TACOperand;

typedef enum {
    TAC_BINARY_OP,    // t = a + b, t = a * b, etc.
    TAC_UNARY_OP,     // t = -a
    TAC_COPY,         // t = a
    TAC_LABEL,        // label:
    TAC_GOTO,         // goto label
    TAC_IFZ,          // ifz cond goto label
    TAC_PARAM,        // param x
    TAC_CALL,         // t = call f, n_args
    TAC_RETURN,       // return t or return
    TAC_FUNCTION,     // fun name
    TAC_END_FUNCTION, // End of function definition
    TAC_DEFINE
} TACOpKind;

typedef enum {
    TAC_ADD, TAC_SUB, TAC_MUL, TAC_DIV, TAC_MOD,
    TAC_EQ, TAC_NEQ, TAC_LT, TAC_LTE, TAC_GT, TAC_GTE,
    TAC_AND, TAC_OR
} TACBinOp;

typedef enum {
    TAC_NEG, TAC_NOT
} TACUnaryOp;


typedef struct TACInstr {
    TACOpKind kind;

    TACOperand *dst;
    TACOperand *arg1;
    TACOperand *arg2; 

    union {
        TACBinOp binop;
        TACUnaryOp unop;
    } op;

    struct TACInstr *next; // Pointer to the next instruction in the list
} TACInstr;
