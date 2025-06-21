#include "ast_print.h"
#include "token_util.h"


// AST printing
static void print_indent(int level)
{
    for (int i = 0; i < level; i++) {
        fputs("  ", stdout);
    }
}


// Helper to print a list of AstNodes
void print_list(AstNode *items[], size_t count, int indent) {
    for (size_t i = 0; i < count; ++i) {
        print_ast(items[i], indent);
    }
}

void print_ast(AstNode *node, int indent) {
    if (!node) return;
    print_indent(indent);
    
    switch (node->type) {
        case AST_BLOCK: {
            puts("Block:");
            print_list((const AstNode *const *)node->data.block.statements,
                       node->data.block.count,
                       indent + 1);
            break;
        }

        case AST_VARIABLE:
            printf("Variable: %s\n", node->data.variable.identifier);
            break;

        case AST_LITERAL:
            printf("IntLiteral: %d\n", node->data.literal.value);
            break;

        case AST_BINARY_OP:
            printf("BinaryOp: %s\n", binaryop_to_string(node->data.binary.op));
            print_ast(node->data.binary.left,  indent + 1);
            print_ast(node->data.binary.right, indent + 1);
            break;

        case AST_UNARY_OP:
            printf("UnaryOp: %s\n", unarop_to_string(node->data.unary.op));
            print_ast(node->data.unary.operand, indent + 1);
            break;

        case AST_DECLARATION:
            puts("Declaration:");
            print_ast(node->data.declaration.variable, indent + 1);
            print_ast(node->data.declaration.value,    indent + 1);
            break;

        case AST_ASSIGNMENT:
            printf("Assignment: %s\n",
                   node->data.assignment.variable->data.variable.identifier);
            print_ast(node->data.assignment.value, indent + 1);
            break;

        case AST_CALL: {
            printf("Call: %s\n",
                   node->data.call.callee->data.variable.identifier);
            print_indent(indent + 1);
            puts("Arguments:");
            print_list(node->data.call.args->data.args.arguments,
                       node->data.call.args->data.args.count,
                       indent + 2);
            break;
        }

        case AST_IF:
            puts("IfStatement:");
            print_indent(indent + 1); puts("Condition:");
            print_ast(node->data.if_stmt.condition, indent + 2);
            print_indent(indent + 1); puts("ThenBlock:");
            print_ast(node->data.if_stmt.then_block, indent + 2);
            if (node->data.if_stmt.else_block) {
                print_indent(indent + 1); puts("ElseBlock:");
                print_ast(node->data.if_stmt.else_block, indent + 2);
            }
            break;

        case AST_WHILE:
            puts("WhileLoop:");
            print_indent(indent + 1); puts("Condition:");
            print_ast(node->data.while_loop.condition, indent + 2);
            print_indent(indent + 1); puts("Body:");
            print_ast(node->data.while_loop.body, indent + 2);
            break;

        case AST_RETURN:
            puts("ReturnStatement:");
            print_ast(node->data.return_stmt.expression, indent + 1);
            break;

        case AST_FUNCTION:
            printf("Function: %s\n", node->data.function.name->data.variable.identifier);
            print_indent(indent + 1); puts("Parameters:");
            print_list((const AstNode *const *)node->data.function.params->data.params.params,
                       node->data.function.params->data.params.count,
                       indent + 2);
            print_indent(indent + 1); puts("Body:");
            print_ast(node->data.function.body, indent + 2);
            break;

        default:
            printf("<Unknown AST node: %s>\n", astnode_type_to_string(node->type));
            break;
    }
}



void print_json_fp(FILE *out, AstNode *n){
    if(!out) return;
    if(!n){ fprintf(out, "null"); return; }
    switch(n->type){
    case AST_BLOCK:
        fprintf(out, "{\"type\":\"Block\",\"stmts\":[");
        for(size_t i=0;i<n->data.block.count;i++){
            if(i) fputc(',', out);
            print_json_fp(out, n->data.block.statements[i]);
        }
        fprintf(out, "]}");
        break;
    case AST_VARIABLE:
        fprintf(out, "{\"type\":\"Variable\",\"name\":\"%s\"}",
                n->data.variable.identifier);
        break;
    case AST_LITERAL:
        fprintf(out, "{\"type\":\"IntLiteral\",\"value\":%d}",
                n->data.literal.value);
        break;
    case AST_BINARY_OP:
        fprintf(out, "{\"type\":\"BinaryOp\",\"op\":\"%s\",\"left\":",
                binaryop_to_string(n->data.binary.op));
        print_json_fp(out, n->data.binary.left);
        fprintf(out, ",\"right\":");
        print_json_fp(out, n->data.binary.right);
        fprintf(out, "}");
        break;
    case AST_UNARY_OP:
        fprintf(out, "{\"type\":\"UnaryOp\",\"op\":\"%c\",\"operand\":",
                n->data.unary.op);
        print_json_fp(out, n->data.unary.operand);
        fprintf(out, "}");
        break;
    case AST_DECLARATION:
        fprintf(out, "{\"type\":\"Declaration\",\"var\":");
        print_json_fp(out, n->data.declaration.variable);
        fprintf(out, ",\"value\":");
        print_json_fp(out, n->data.declaration.value);
        fprintf(out, "}");
        break;
    case AST_ASSIGNMENT:
        fprintf(out, "{\"type\":\"Assignment\",\"var\":\"%s\",\"value\":",
                n->data.assignment.variable->data.variable.identifier);
        print_json_fp(out, n->data.assignment.value);
        fprintf(out, "}");
        break;
    case AST_CALL:
        fprintf(out, "{\"type\":\"Call\",\"callee\":\"%s\",\"args\":[",
                n->data.call.callee->data.variable.identifier);
        for(size_t i=0;i<n->data.call.args->data.args.count;i++){
            if(i) fputc(',', out);
            print_json_fp(out, n->data.call.args->data.args.arguments[i]);
        }
        fprintf(out, "]}");
        break;
    case AST_IF:
        fprintf(out, "{\"type\":\"If\",\"cond\":");
        print_json_fp(out, n->data.if_stmt.condition);
        fprintf(out, ",\"then\":");
        print_json_fp(out, (AstNode*)n->data.if_stmt.then_block);
        if(n->data.if_stmt.else_block){
            fprintf(out, ",\"else\":");
            print_json_fp(out, (AstNode*)n->data.if_stmt.else_block);
        }
        fprintf(out, "}");
        break;
    case AST_WHILE:
        fprintf(out, "{\"type\":\"While\",\"cond\":");
        print_json_fp(out, n->data.while_loop.condition);
        fprintf(out, ",\"body\":");
        print_json_fp(out, (AstNode*)n->data.while_loop.body);
        fprintf(out, "}");
        break;
    case AST_RETURN:
        fprintf(out, "{\"type\":\"Return\",\"expr\":");
        print_json_fp(out, n->data.return_stmt.expression);
        fprintf(out, "}");
        break;
    case AST_FUNCTION:
        fprintf(out, "{\"type\":\"Function\",\"name\":\"%s\",\"params\":[",
                n->data.function.name->data.variable.identifier);
        for(size_t i=0;i<n->data.function.params->data.params.count;i++){
            if(i) fputc(',', out);
            print_json_fp(out, n->data.function.params->data.params.params[i]);
        }
        fprintf(out, "],\"body\":");
        print_json_fp(out, (AstNode*)n->data.function.body);
        fprintf(out, "}");
        break;
    default:
        fprintf(out, "\"Unknown\"");
    }
}



void dump_ast_json_file(const char *filename, AstNode *root) {
    FILE *out = NULL;
    if (!filename || strcmp(filename, "-") == 0) {
        out = stdout;
    } else {
        out = fopen(filename, "w");
        if (!out) { perror("fopen"); return; }
    }
    print_json_fp(out, root);
    if (out != stdout) fclose(out);
}

