#include "parser.h"
#include "parse_error.h"
#include <stdio.h>
#include <stdlib.h>

#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BOLD    "\033[1m"
#define COLOR_BOLD_RED    COLOR_BOLD COLOR_RED
#define COLOR_BOLD_YELLOW COLOR_BOLD COLOR_YELLOW

// Error-reporting and exit
void parse_error(const Parser *parser,
                 TokenType expected,
                 const Token *actual)
{   
    FILE *file = fopen(parser->filename, "r");
    // Read the line where the error occurred
    if (!file) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    char line_str[256];
    int line_number = actual->line;
    for (int i = 1; i <= line_number; ++i) {
        if (!fgets(line_str, sizeof(line_str), file)) {
            fprintf(stderr, "Error reading file: %s\n", parser->filename);
            fclose(file);
            exit(EXIT_FAILURE);
        }
    }
    fclose(file);

    fprintf(stderr, "%s:%d:%d: parse error:\n", 
            parser->filename,
            actual->line,
            actual->column);

    // Print the line of code where the error occurred
    fprintf(stderr, "    %s\n", line_str);


    // Print a caret pointing to the error column
    fprintf(stderr, "    ");
    for (int i = 1; i < actual->column; ++i) {
        fputc(line_str[i - 1] == '\t' ? '\t' : ' ', stderr);
    }
    fprintf(stderr, "^\n");

    // Show expected vs actual token
    fprintf(stderr, "    Expected token: <%s>\n", token_type_to_string(expected));
    fprintf(stderr, "    Actual token  : <%s> ('%s')\n",
            token_type_to_string(actual->type),
            actual->value ? actual->value : "");

    exit(EXIT_FAILURE);
}



void read_line_from_file(const char *filename, int line_number, char *buffer, size_t buffer_size) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    for (int i = 1; i <= line_number; ++i) {
        if (!fgets(buffer, buffer_size, file)) {
            fprintf(stderr, "Error reading file: %s\n", filename);
            fclose(file);
            exit(EXIT_FAILURE);
        }
    }
    fclose(file);
}

void print_caret(int column) {
    for (int i = 1; i < column; ++i) {
        fputc(' ', stderr);
    }
    fputs("^", stderr);
    fputc('\n', stderr);
}

void report_parse_error(ParseError *err) {
    fprintf(stderr, COLOR_BOLD_RED "%s:%d:%d: error:" COLOR_RESET " %s\n",
            err->filename, err->line, err->column, err->message);

    char line_buffer[256];
    read_line_from_file(err->filename, err->line, line_buffer, sizeof(line_buffer));
    fprintf(stderr, "%s", line_buffer);
    print_caret(err->column);

    if (err->expected) {
        fprintf(stderr, COLOR_YELLOW "expected:" COLOR_RESET " %s\n", err->expected);
    }

    if (err->found) {
        fprintf(stderr, COLOR_YELLOW "found:" COLOR_RESET " %s\n", err->found);
    }


    if (err->is_fatal) {
        exit(1);
    }
}

ParseError *create_parse_error(int line, int column, const char *filename,
                        const char *message,
                        const char *expected, const char *found,
                        int is_fatal) {
    
    ParseError *err = malloc(sizeof(ParseError));
    err->line = line;
    err->column = column;
    err->filename = filename;
    err->message = message;
    err->expected = expected;
    err->found = found;
    err->is_fatal = is_fatal;
    return err;
}