#include "lexer.h"
#include "regex_patterns.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Token classification for keywords
static const struct { const char *name; TokenType type; } keywords[] = {
    {"def",    TOKEN_DEFINE},
    {"fn",     TOKEN_FUNCTION},
    {"if",     TOKEN_IF},
    {"else",   TOKEN_ELSE},
    {"return", TOKEN_RETURN},
};
static const size_t keyword_count = sizeof(keywords) / sizeof(keywords[0]);


// Regex rules (compiled once)
static int patterns_ready = 0;
static regex_t re_delimiter;   // newline or whitespace
static regex_t re_identifier;
static regex_t re_number;
static regex_t re_operator;
static regex_t re_string;
static regex_t re_paren_open;
static regex_t re_paren_close;
static regex_t re_brace_open;
static regex_t re_brace_close;
static regex_t re_comparison;
static regex_t re_logical;
static regex_t re_endln;


// Try each token type
static const struct { regex_t *re; TokenType type; } tests[] = {
    {&re_paren_open,   TOKEN_PAREN_OPEN},
    {&re_paren_close,  TOKEN_PAREN_CLOSE},
    {&re_brace_open,   TOKEN_BRACE_OPEN},
    {&re_brace_close,  TOKEN_BRACE_CLOSE},
    {&re_comparison,   TOKEN_OPERATOR},
    {&re_logical,      TOKEN_LOGICAL},
    {&re_identifier,   TOKEN_IDENTIFIER},
    {&re_number,       TOKEN_NUMBER},
    {&re_string,       TOKEN_STRING},
    {&re_operator,     TOKEN_OPERATOR},
    {&re_endln,        TOKEN_END_OF_LINE},
};

static const size_t test_count = sizeof(tests) / sizeof(tests[0]);


// Compile all patterns (called at startup)
void lexer_init_patterns(void) {
    if (regcomp(&re_delimiter, "^[ \t\r\n]+", REG_EXTENDED) != 0 ||
        regcomp(&re_identifier, "^" REGEX_IDENTIFIER, REG_EXTENDED) != 0 ||
        regcomp(&re_paren_open,  "^" REGEX_PAREN_OPEN, REG_EXTENDED) != 0 ||
        regcomp(&re_paren_close, "^" REGEX_PAREN_CLOSE, REG_EXTENDED) != 0 ||
        regcomp(&re_brace_open,  "^" REGEX_BRACE_OPEN, REG_EXTENDED) != 0 ||
        regcomp(&re_brace_close, "^" REGEX_BRACE_CLOSE, REG_EXTENDED) != 0 ||
        regcomp(&re_comparison,  "^" REGEX_COMPARISON, REG_EXTENDED) != 0 ||
        regcomp(&re_logical,     "^" REGEX_LOGICAL, REG_EXTENDED) != 0 ||
        regcomp(&re_endln,       "^" REGEX_END_OF_LINE, REG_EXTENDED) != 0 ||
        regcomp(&re_number,     "^" REGEX_NUMBER,     REG_EXTENDED) != 0 ||
        regcomp(&re_operator,   "^" REGEX_OPERATOR,   REG_EXTENDED) != 0) {
        fprintf(stderr, "Failed to compile regex patterns\n");
        exit(EXIT_FAILURE);
    }
    patterns_ready = 1;
}

// Release compiled patterns (called at shutdown)
void lexer_free_patterns(void) {
    if (!patterns_ready) return;
    regfree(&re_delimiter);
    regfree(&re_endln);
    regfree(&re_identifier);
    regfree(&re_number);
    regfree(&re_operator);
    regfree(&re_string);
    regfree(&re_paren_open);
    regfree(&re_paren_close);
    regfree(&re_brace_open);
    regfree(&re_brace_close);
    regfree(&re_comparison);
    regfree(&re_logical);
    patterns_ready = 0;
}

// Advance cursor and update position
static void advance(Lexer *lx, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        if (lx->cursor[i] == '\n') {
            lx->line++;
            lx->column = 1;
        } else {
            lx->column++;
        }
    }
    lx->cursor += n;
}

// Determine if an identifier is a keyword
static TokenType classify_keyword_len(const char *s, size_t len) {
    for (size_t i = 0; i < keyword_count; ++i) {
        if (strlen(keywords[i].name) == len &&
            strncmp(s, keywords[i].name, len) == 0)
            return keywords[i].type;
    }
    return TOKEN_IDENTIFIER;
}


Lexer *lexer_create(const char *source) {
    if (!patterns_ready) lexer_init_patterns();
    Lexer *lx = malloc(sizeof *lx);
    if (!lx) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    lx->source = lx->cursor = source;
    lx->line = lx->column = 1;
    return lx;
}
/**
 * Advance the lexer and return the next token from the input.
 *
 * The lexer rules can be easily expanded internally.
 *
 * @param lx
 *   Pointer to a Lexer instance. 
 *
 * @return
 *   A pointer to a newly allocated Token describing the next lexeme.
 *   The caller is responsible for freeing this token (e.g. via
 *   free_token()).  When the input is exhausted, returns a TOKEN_EOF.
 */
Token *lexer_next(Lexer *lx) {
    if (!*lx->cursor) {
        return create_token(TOKEN_EOF, "", 0, lx->line, lx->column);
    }

    regmatch_t m;
    // Skip delimiters
    while (*lx->cursor && regexec(&re_delimiter, lx->cursor, 1, &m, 0) == 0) {
        advance(lx, m.rm_eo);
    }

    // EOF after skipping
    if (!*lx->cursor) {
        return create_token(TOKEN_EOF, "", 0, lx->line, lx->column);
    }

    for (size_t i = 0; i < test_count; ++i) {
        if (regexec(tests[i].re, lx->cursor, 1, &m, 0) == 0) {
            size_t len = m.rm_eo;
            TokenType type = tests[i].type;
            if (type == TOKEN_IDENTIFIER)
                type = classify_keyword_len(lx->cursor, len);
            Token *tok = create_token(type, lx->cursor, len, lx->line, lx->column);
            advance(lx, len);
            return tok;
        }
    }

    // Unknown single character
    char unknown[2] = {*lx->cursor, '\0'};
    Token *tok = create_token(TOKEN_UNKNOWN, unknown, 1, lx->line, lx->column);
    advance(lx, 1);
    return tok;
}

void free_lexer(Lexer *lx) {
    if (!lx) return;
    free(lx);
}

