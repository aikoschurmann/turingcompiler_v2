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
x = 42;
fn add(a, b) {
    return a + b;
}
if (x < 10) {
    x = add(x, 1);
} else {
    x = add(x, -1);
}
```

# Output (AST):

```yaml
Block:
  Assignment: x
    IntLiteral: 42
  Function: add
    Parameters:
      Variable: a
      Variable: b
    Body:
      Block:
        ReturnStatement:
          BinaryOp: +
            Variable: a
            Variable: b
  IfStatement:
    Condition:
      BinaryOp: <
        Variable: x
        IntLiteral: 10
    ThenBlock:
      Block:
        Assignment: x
          Call: add
            Arguments:
              Variable: x
              IntLiteral: 1
    ElseBlock:
      Block:
        Assignment: x
          Call: add
            Arguments:
              Variable: x
              UnaryOp: -
                IntLiteral: 1
```
# Output (TAC):

```c
x = 42
fun add:
param a
param b
t0 = a + b
return t0
endfun
t1 = x < 10
ifz t1 goto L2
param x
param 1
t4 = call add 2
x = t4
goto L3
L2:
param x
t5 = -1
param t5
t6 = call add 2
x = t6
L3:
```