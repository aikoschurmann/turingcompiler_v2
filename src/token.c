#include "token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Token *create_token(TokenType type, const char *value, size_t len, int line, int column) {
    Token *tok = malloc(sizeof *tok);
    if (!tok) exit(1);
    tok->type   = type;
    tok->value  = strndup(value, len); // only one allocation
    tok->line   = line;
    tok->column = column;
    return tok;
}


void free_token(Token *tok) {
    if (!tok) return;
    free(tok->value);
    free(tok);
}

const char *token_type_to_string(TokenType t) {
    switch (t) {
        case TOKEN_DEFINE:      return "DEFINE";
        case TOKEN_FUNCTION:    return "FUNCTION";
        case TOKEN_IF:          return "IF";
        case TOKEN_ELSE:        return "ELSE";
        case TOKEN_RETURN:      return "RETURN";
        case TOKEN_IDENTIFIER:  return "IDENTIFIER";
        case TOKEN_NUMBER:      return "NUMBER";
        case TOKEN_STRING:      return "STRING";
        case TOKEN_OPERATOR:    return "OPERATOR";
        case TOKEN_COMPARISON:  return "COMPARISON";
        case TOKEN_LOGICAL:     return "LOGICAL";
        case TOKEN_PAREN_OPEN:  return "PAREN_OPEN";
        case TOKEN_PAREN_CLOSE: return "PAREN_CLOSE";
        case TOKEN_BRACE_OPEN:  return "BRACE_OPEN";
        case TOKEN_BRACE_CLOSE: return "BRACE_CLOSE";
        case TOKEN_UNKNOWN:     return "UNKNOWN";
        case TOKEN_EOF:         return "EOF";
        case TOKEN_END_OF_LINE: return "EOL";
    }
    return "<?>";
}


void print_token(const Token *tok) {
    printf("<%s: \"%s\"> at %d:%d\n",
           token_type_to_string(tok->type),
           tok->value,
           tok->line,
           tok->column);
}

#define COLOR_RESET   "\x1b[0m"
#define COLOR_TYPE    "\x1b[1;34m"  // bold blue
#define COLOR_VALUE   "\x1b[0;32m"  // green
#define COLOR_POS     "\x1b[0;37m"  // light gray

void print_token_colored(const Token *tok) {
    printf(COLOR_TYPE "<%s>" COLOR_RESET " " 
           COLOR_VALUE "\"%s\"" COLOR_RESET " " 
           COLOR_POS "%d:%d" COLOR_RESET "\n",
           token_type_to_string(tok->type),
           tok->value,
           tok->line, tok->column);
}

void token_array_init(TokenArray *arr) {
    arr->data = NULL;
    arr->size = arr->capacity = 0;
}

void token_array_push(TokenArray *arr, Token *tok) {
    if (arr->size + 1 > arr->capacity) {
        arr->capacity = arr->capacity ? arr->capacity*2 : 8;
        arr->data = realloc(arr->data, sizeof *arr->data * arr->capacity);
        if (!arr->data) {
            perror("realloc");
            exit(1);
        }
    }
    arr->data[arr->size++] = tok;
}

void token_array_free(TokenArray *arr) {
    for (size_t i = 0; i < arr->size; i++) {
        free_token(arr->data[i]);
    }
    free(arr->data);
    arr->data = NULL;
    arr->size = arr->capacity = 0;
}

/**
 * Dumps an array of tokens as JSON to the given FILE* stream.
 *
 * @param out      The output stream (e.g. stdout or a file opened for writing).
 * @param tokens   Array of Token* pointers.
 * @param n        Number of tokens in the array.
 */
void dump_tokens_json_fp(FILE *out, Token **tokens, size_t n) {
    if (!out) return;
    fprintf(out, "[\n");
    for (size_t i = 0; i < n; i++) {
        Token *t = tokens[i];
        fprintf(out,
                "  { \"type\": \"%s\", \"value\": \"%s\", \"line\": %d, \"col\": %d }%s\n",
                token_type_to_string(t->type),
                t->value ? t->value : "",
                t->line,
                t->column,
                (i + 1 < n) ? "," : "");
    }
    fprintf(out, "]\n");
}

/**
 * Convenience wrapper: dumps tokens to a file specified by name.
 * If filename is NULL or "-", writes to stdout.
 *
 * @param filename The path of the file to write, or "-"/NULL for stdout.
 * @param tokens   Array of Token* pointers.
 * @param n        Number of tokens in the array.
 */
void dump_tokens_json_file(const char *filename, Token **tokens, size_t n) {
    FILE *out = NULL;
    if (!filename || strcmp(filename, "-") == 0) {
        out = stdout;
    } else {
        out = fopen(filename, "w");
        if (!out) {
            perror("fopen");
            return;
        }
    }

    dump_tokens_json_fp(out, tokens, n);

    if (out != stdout) {
        fclose(out);
    }
}