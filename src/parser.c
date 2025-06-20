#include "parser.h"
#include "parse_error.h"


Token *current_token(Parser *p) {
    if (p->current >= p->end) {
        static Token eof = { .type = TOKEN_EOF };
        return &eof;
    }
    return p->tokens.data[p->current];
}

Token *peek(Parser *p, size_t offset) {
    size_t idx = p->current + offset;
    return (idx < p->end) ? p->tokens.data[idx] : NULL;
}

Token *consume(Parser *p, TokenType expected, const char *value) {
    Token *tok = current_token(p);
    if (tok->type != expected) parse_error(p, expected, tok);
    if (value && strcmp(tok->value, value) != 0) parse_error(p, expected, tok);
    p->current++;
    return tok;
}

// Parser lifecycle
Parser *parser_create(TokenArray tokens, const char *filename)
{
    Parser *p = malloc(sizeof(*p));
    if (!p) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    p->tokens   = tokens;
    p->start    = 0;                 // cover the whole stream
    p->end      = tokens.size;
    p->current  = p->start;
    p->filename = strdup(filename);
    return p;
}

Parser parser_slice(const Parser *orig,
                    size_t slice_start,
                    size_t slice_end)
{
    Parser slice = *orig;       // shallow copy
    slice.start   = slice_start;
    slice.current = slice_start;
    slice.end     = slice_end;
    return slice;
}


void parser_free(Parser *parser)
{
    if (!parser) return;
    token_array_free(&parser->tokens);
    free(parser->filename);
    free(parser);
}

