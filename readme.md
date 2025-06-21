# Mini‑Language Compiler

A small compiler front‑end for a C‑like “mini‑language.” It performs lexical analysis (lexer), parsing into an AST, and provides both a pretty‑print and a JSON dump of the AST.

---

## Table of Contents

- Features  
- Project Structure  
- Building  
- Example  

---

## Features

- **Lexer**  
  - Tokenizes keywords (`def`, `fn`, `if`, `else`, `while`, `return`), identifiers, numbers, operators, delimiters, parentheses & braces, strings, commas, and end‑of‑line markers.  
  - Uses POSIX regex for pattern matching.  

- **Parser**  
  - Combination of Recursive Descent and Pratt Parsing
  - Statement parsing: variable declarations, assignments, `if`/`else`, `while`, function definitions, `return` statements, and block grouping.  

- **AST Output**  
  - Human‑readable tree printer  
  - JSON emitter (`dump_ast_json_file`)  

---

## Project Structure

- **`lexer.*`** – regex‑based tokenizer (`lexer_init_patterns`, `lexer_next`, `lexer_free_patterns`)  
- **`parser.*`** – top‑level parse driver & parser lifecycle (`parser_create`, `parse`, `parser_free`)  
- **`parse_statement.*`** – per‑statement parsing routines (`parse_declaration`, `parse_if_statement`, etc.)  
- **`parse_error.*`** – error reporting with source‑line context (`parse_error`, `report_fatal_parse_error`)  
- **`ast.*`** – AST structs, creation, memory management (`ast_create_node`, `free_ast_node`)  
- **`ast_print.*`** – AST printing & JSON serialization (`print_ast`, `dump_ast_json_file`)  
- **`token_util.*`** – enum‑to‑string helpers and operator maps (`astnode_type_to_string`, `binaryop_to_string`, `token_type_to_string`, `is_prefix_op`)  
- **`pratt_parse.*`** – Pratt parser for precedence & infix/prefix operators  
- **`token.h`** – definitions of `Token`, `TokenType`, and `TokenArray`  

---

## Building

You’ll need a C compiler (e.g. `gcc` or `clang`) and POSIX regex support.


## Example
# Example Mini‑Language Program


```c
fn factorial(n) {
    def result = 1;
    def i = 1;

    while (i <= n) {
        result = result * i;
        i = i + 1;
    }

    return result;
}

fn main() {
    def n = 5;
    def res = factorial(n);

    return res;
}

```
# Output (LEX):
```c
<FUNCTION> "fn" 1:1
<IDENTIFIER> "factorial" 1:4
<PAREN_OPEN> "(" 1:13
<IDENTIFIER> "n" 1:14
<PAREN_CLOSE> ")" 1:15
<BRACE_OPEN> "{" 1:17
<DEFINE> "def" 4:5
<IDENTIFIER> "result" 4:9
<OPERATOR> "=" 4:16
<NUMBER> "1" 4:18
<EOL> ";" 4:19
<DEFINE> "def" 5:5
<IDENTIFIER> "i" 5:9
<OPERATOR> "=" 5:11
<NUMBER> "1" 5:13
<EOL> ";" 5:14
<WHILE> "while" 7:5
<PAREN_OPEN> "(" 7:11
<IDENTIFIER> "i" 7:12
<OPERATOR> "<=" 7:14
<IDENTIFIER> "n" 7:17
<PAREN_CLOSE> ")" 7:18
<BRACE_OPEN> "{" 7:20
<IDENTIFIER> "result" 8:9
<OPERATOR> "=" 8:16
<IDENTIFIER> "result" 8:18
<OPERATOR> "*" 8:25
<IDENTIFIER> "i" 8:27
<EOL> ";" 8:28
<IDENTIFIER> "i" 9:9
<OPERATOR> "=" 9:11
<IDENTIFIER> "i" 9:13
<OPERATOR> "+" 9:15
<NUMBER> "1" 9:17
<EOL> ";" 9:18
<BRACE_CLOSE> "}" 10:5
<RETURN> "return" 12:5
<IDENTIFIER> "result" 12:12
<EOL> ";" 12:18
<BRACE_CLOSE> "}" 13:1
<FUNCTION> "fn" 15:1
<IDENTIFIER> "main" 15:4
<PAREN_OPEN> "(" 15:8
<PAREN_CLOSE> ")" 15:9
<BRACE_OPEN> "{" 15:11
<DEFINE> "def" 16:5
<IDENTIFIER> "n" 16:9
<OPERATOR> "=" 16:11
<NUMBER> "5" 16:13
<EOL> ";" 16:14
<DEFINE> "def" 17:5
<IDENTIFIER> "res" 17:9
<OPERATOR> "=" 17:13
<IDENTIFIER> "factorial" 17:15
<PAREN_OPEN> "(" 17:24
<IDENTIFIER> "n" 17:25
<PAREN_CLOSE> ")" 17:26
<EOL> ";" 17:27
<RETURN> "return" 19:5
<IDENTIFIER> "res" 19:12
<EOL> ";" 19:15
<BRACE_CLOSE> "}" 20:1
<EOF> "" 21:1
```

# Output (AST):

```yaml
Block:
  Function: factorial
    Parameters:
      Variable: n
    Body:
      Block:
        Declaration:
          Variable: result
          IntLiteral: 1
        Declaration:
          Variable: i
          IntLiteral: 1
        WhileLoop:
          Condition:
            BinaryOp: <=
              Variable: i
              Variable: n
          Body:
            Block:
              Assignment: result
                BinaryOp: *
                  Variable: result
                  Variable: i
              Assignment: i
                BinaryOp: +
                  Variable: i
                  IntLiteral: 1
        ReturnStatement:
          Variable: result
  Function: main
    Parameters:
    Body:
      Block:
        Declaration:
          Variable: n
          IntLiteral: 5
        Declaration:
          Variable: res
          Call: factorial
            Arguments:
              Variable: n
        ReturnStatement:
          Variable: res
```
# Output (TAC):

```c
   1: fun factorial:
   2:   param n
   3:   result ← 1
   4:   i ← 1
   5:   L0:
   6:   t1 ← i <= n
   7:   ifz t1 goto L2
   8:     t3 ← result * i
   9:     result ← t3
  10:     t4 ← i + 1
  11:     i ← t4
  12:     goto L0
  13:   L2:
  14:   t5 ← result
  15:   return t5
  16: endfun
  17: fun main:
  18:   n ← 5
  19:   param n
  20:   t6 ← call factorial 1
  21:   res ← t6
  22:   t7 ← res
  23:   return t7
  24: endfun
```