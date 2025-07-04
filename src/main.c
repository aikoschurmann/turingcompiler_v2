#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler.h"
#include "token_util.h"


int main(void) {
    const char *code = read_file("./input/test.txt");

    /* 1) init lexer and token array */
    Lexer *lx = lexer_create(code);
    TokenArray tokens;
    token_array_init(&tokens);

    /* 2) lex the input */
    Token *tok;
    while ((tok = lexer_next(lx))->type != TOKEN_EOF) {
        token_array_push(&tokens, tok);
    }
    /* also store the EOF */
    token_array_push(&tokens, tok);

    free_lexer(lx);

    /* 3) print the tokens */
    //for (size_t i = 0; i < tokens.size; i++) {
    //    print_token_colored(tokens.data[i]);
    //}
    //printf("\n\n");

    dump_tokens_json_file("./compiler-steps/tokens.json", tokens.data, tokens.size);
    
    // 3.5) parse the tokens 
    Parser *parser = parser_create(tokens, "./input/test.txt");
    AstNode *ast = parse(parser);   
    //print_ast(ast, 0);

    printf("\n\n");
    dump_ast_json_file("./compiler-steps/ast.json", ast);
    FILE *out = stdout;

    int temp_counter = 0;
    TACInstr *instr = tac_parse(ast, &temp_counter);
    //tac_print_list(instr);
    CFG *cfg2 = extract_functions(instr);
    print_cfg(cfg2);
    //CFG *cfg = build_from_tac(instr);
    //print_cfg(cfg);


    /* 4) cleanup */
    parser_free(parser);
    free_ast_node(ast);

    return 0;
}
