// error.h
#pragma once

typedef struct {
    int line;
    int column;
    const char *filename;

    const char *message;
    const char *expected;
    const char *found;

    int is_fatal;
} ParseError;

void parse_error(const Parser *parser,
                 TokenType expected,
                 const Token *actual);
void report_parse_error(ParseError *err);

ParseError *create_parse_error(
                      int line, int column, const char *filename,
                      const char *message,
                      const char *expected, const char *found,
                      int is_fatal);
