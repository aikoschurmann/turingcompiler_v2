#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Token types
typedef enum {
    TOKEN_DEFINE,
    TOKEN_FUNCTION,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_RETURN,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_OPERATOR,
    TOKEN_COMPARISON,
    TOKEN_LOGICAL,
    TOKEN_PAREN_OPEN,
    TOKEN_PAREN_CLOSE,
    TOKEN_BRACE_OPEN,
    TOKEN_BRACE_CLOSE,
    TOKEN_COMMA,
    TOKEN_UNKNOWN,
    TOKEN_EOF,
    TOKEN_END_OF_LINE,
    TOKEN_WHILE
} TokenType;


// Token struct
typedef struct {
    TokenType type;
    char     *value;
    int       line;
    int       column;
} Token;


/* A growable array of Token pointers */
typedef struct {
    Token **data;
    size_t size, capacity;
} TokenArray;


// Token utilities
Token *create_token(TokenType type, const char *value, size_t len, int line, int column);
void   free_token(Token *tok);
const char *token_type_to_string(TokenType t);
void   print_token(const Token *tok);
void   print_token_colored(const Token *tok);
void   token_array_init(TokenArray *arr);
void   token_array_push(TokenArray *arr, Token *tok);
void   token_array_free(TokenArray *arr);
void   dump_tokens_json_fp(FILE *out, Token **tokens, size_t n);
void   dump_tokens_json_file(const char *filename, Token **tokens, size_t n);
