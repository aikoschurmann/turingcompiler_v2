#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "file.h"
#include "parser.h"

#include <stdio.h>



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
    for (size_t i = 0; i < tokens.size; i++) {
        print_token_colored(tokens.data[i]);
    }
    printf("\n\n");


    dump_tokens_json_file("./compiler-steps/tokens.json", tokens.data, tokens.size);
    
    // 3.5) parse the tokens 
    Parser *parser = parser_create(tokens, "test.txt");
    AstNode *ast = parse(parser);   
    print_ast(ast, 0);
    printf("\n\n");

    /* 4) cleanup */
    parser_free(parser);
    free_ast_node(ast);

    return 0;
}
