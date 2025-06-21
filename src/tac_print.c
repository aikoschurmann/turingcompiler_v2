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
      case TAC_OP_LABEL:   printf("L%d",   op->literal); break;
      default:             printf("<?>");                break;
    }
}

/* Print a single TAC instruction */
void tac_print_instr(const TACInstr *p) {
    if (!p) return;

    switch (p->kind) {
      case TAC_BINARY_OP:
        tac_print_operand(p->dst); printf(" ← ");
        tac_print_operand(p->arg1); printf(" %s ", tac_binop_str(p->op.binop));
        tac_print_operand(p->arg2); printf("\n");
        break;

      case TAC_UNARY_OP:
        tac_print_operand(p->dst); printf(" ← %s ", tac_unop_str(p->op.unop));
        tac_print_operand(p->arg1); printf("\n");
        break;

      case TAC_COPY:
        tac_print_operand(p->dst); printf(" ← ");
        tac_print_operand(p->arg1); printf("\n");
        break;

      case TAC_LABEL:
        if (p->dst)
            printf("L%d:\n", p->dst->literal);
        else
            printf("L<?>:\n");
        break;

      case TAC_GOTO:
        if (p->arg1)
            printf("goto L%d\n", p->arg1->literal);
        else
            printf("goto L<?>?\n");
        break;

      case TAC_IFZ:
        if (p->arg1 && p->arg2)
            printf("ifz t%d goto L%d\n", p->arg1->literal, p->arg2->literal);
        else
            printf("ifz ? goto ?\n");
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
        if (p->dst)
            printf("fun %s:\n", p->dst->name);
        else
            printf("fun <?>:\n");
        break;

      case TAC_PUSH:
        printf("push ");
        tac_print_operand(p->arg1);
        printf("\n");
        break;
      case TAC_POP:
        printf("pop ");
        tac_print_operand(p->arg1);
        printf("\n");
        break;

      case TAC_CALL:
        printf("t%d ← call %s %d\n",
               p->dst ? p->dst->literal : -1,
               p->arg1 ? p->arg1->name : "<??>",
               p->arg2 ? p->arg2->literal : 0);
        break;

      case TAC_END_FUNCTION:
        printf("endfun\n\n");
        break;
      case TAC_DEFINE:
          if (p->dst) {
              printf("define %s", p->dst->name);
              if (p->arg1) {
                  // print the initial value
                  printf(" = ");
                  tac_print_operand(p->arg1);
              }
              printf("\n");
          } else {
              printf("define <?>\n");
          }
          break;

      

      default:
        printf("; [unrecognized TAC kind %d]\n", p->kind);
        break;
    }
}

/* Internal helper for label stack */
typedef struct {
    int labels[32];
    int top;
} LabelStack;

static void label_stack_push(LabelStack *s, int label) {
    if (s->top < 32) s->labels[s->top++] = label;
}

static int label_stack_peek(LabelStack *s) {
    return s->top > 0 ? s->labels[s->top - 1] : -1;
}

static void label_stack_pop(LabelStack *s) {
    if (s->top > 0) s->top--;
}

/* Iterate and print a list with line numbers and nested indent for IF/ELSE */
void tac_print_list(TACInstr *head) {
    int lineno = 1;
    int indent_level = 0;
    LabelStack label_stack = {0};

    for (const TACInstr *p = head; p; p = p->next, ++lineno) {
        /* Handle label ending an IF block */
        if (p->kind == TAC_LABEL && label_stack.top > 0 &&
            p->dst && p->dst->literal == label_stack_peek(&label_stack)) {
            label_stack_pop(&label_stack);
            indent_level--;
        }



        
        if (p->kind == TAC_END_FUNCTION) {
            indent_level--;
        }

        /* Print line number and indent */
        printf("%4d: ", lineno);
        for (int i = 0; i < indent_level; ++i)
            printf("  ");

        tac_print_instr(p);

        /* Increase indent for new blocks */
        if (p->kind == TAC_IFZ && p->arg2) {
            label_stack_push(&label_stack, p->arg2->literal);
            indent_level++;
        } else if (p->kind == TAC_FUNCTION) {
            indent_level++;
        }
    }
}


