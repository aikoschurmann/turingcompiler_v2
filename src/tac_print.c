#include "tac_print.h"
#include "tac_util.h"
#include <stdio.h>

/* Formatting for operands */
void tac_print_operand(const TACOperand *op) {
    if (!op) return;
    switch (op->type) {
      case TAC_OP_TEMP:    printf("t%d",   op->literal); break;
      case TAC_OP_VAR:     printf("%s",    op->name);    break;
      case TAC_OP_LITERAL: printf("%d",    op->literal); break;
      case TAC_OP_LABEL:   printf("%d",    op->literal); break;
      default:             printf("<?>",   op->literal);
    }
}

/* Print a single TAC instruction */
void tac_print_instr(const TACInstr *p) {
    if (!p) return;
    switch (p->kind) {
      case TAC_BINARY_OP:
        tac_print_operand(p->dst); printf(" = ");
        tac_print_operand(p->arg1); printf(" %s ", tac_binop_str(p->op.binop));
        tac_print_operand(p->arg2); printf("\n");
        break;
      case TAC_UNARY_OP:
        tac_print_operand(p->dst); printf(" = %s", tac_unop_str(p->op.unop));
        tac_print_operand(p->arg1); printf("\n");
        break;
      case TAC_COPY:
        tac_print_operand(p->dst); printf(" = ");
        tac_print_operand(p->arg1); printf("\n");
        break;
      case TAC_LABEL:
        printf("L%d:\n", p->dst->literal);
        break;
      case TAC_GOTO:
        printf("goto L");
        tac_print_operand(p->arg1); printf("\n");
        break;
      case TAC_IFZ:
        printf("ifz ");
        tac_print_operand(p->arg1); printf(" goto L");
        tac_print_operand(p->arg2); printf("\n");
        break;
      case TAC_RETURN:
        if (p->arg1) {
            printf("return ");
            tac_print_operand(p->arg1);
            printf("\n");
        } else {
            printf("return\n");
        }
        break;
      case TAC_FUNCTION:
        printf("fun %s:\n", p->dst->name);
        break;
      case TAC_PARAM:
        printf("param ");
        tac_print_operand(p->arg1); printf("\n");
        break;
      case TAC_CALL:
        tac_print_operand(p->dst); printf(" = call");
        if (p->arg1) {
            printf(" %s", p->arg1->name);
        }
        if (p->arg2 && p->arg2->type == TAC_OP_LITERAL) {
            printf(" %d\n", p->arg2->literal);
        } else {
            printf("\n");
        }
        break;
      case TAC_END_FUNCTION:
        printf("endfun\n");
        break;
      /* placeholders for future kinds:
      case TAC_ASSIGN:   // print dst = src
      case TAC_LABEL:    // print label:
      case TAC_GOTO:     // print goto label
      case TAC_IFZ:      // print ifz arg1 goto label
      */
      default:
        /* skip unknown kinds */
        break;
    }
}

/* Iterate and print a list */
void tac_print_list(TACInstr *head) {
    for (TACInstr *p = head; p; p = p->next) {
        tac_print_instr(p);
    }
}