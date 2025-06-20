#pragma once

#include "token.h"


typedef struct {
    TokenArray tokens;
    size_t       start;     // new: index of first token in this sub‑parser
    size_t       end;       // new: one past the last token to parse
    size_t       current;   // always lives in [start..end]
    char        *filename;
} Parser;


Parser *parser_create(TokenArray tokens, const char *filename);

Token *consume(Parser *p, TokenType expected, const char *value);

Token *current_token(Parser *p);

Token *peek(Parser *p, size_t offset);

Parser parser_slice(const Parser *orig, size_t slice_start, size_t slice_end);

void parser_free(Parser *parser);

