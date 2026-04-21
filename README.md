# Edu Compiler — C Language Compiler with Phase-by-Phase Visualization
 
A fully working **C-language compiler** built from scratch using Flex, Bison, and C on Linux/WSL. Covers all six classical compilation phases. Each phase has two implementations: a **standalone visualizer** (prints that phase's output independently) and a **chained version** (feeds into the next phase).
 
9 test programs included in `tests/`.
 
---
 
## Compilation Phases
 
```
Source Code (.c / .txt)
        │
        ▼
  [ Lexer ]                    Tokenizes source into categorized lexemes
  lex_visualizer.l             → standalone: prints token table
  lexer_parser_visualizer.l   → chained: feeds token stream to parser
        │
        ▼
  [ Parser ]                   Validates grammar, builds AST
  parser_visualizer.y          → standalone: prints construct table
  parser_semantic.y            → chained: feeds AST to semantic phase
        │
        ▼
  [ Semantic Analysis ]        Type checking, scope, duplicate detection
  semantic_icg.c               → runs checks, then calls ICG if clean
  symbol_table.c               → symbol table implementation
        │
        ▼
  [ ICG ]                      Generates three-address code (TAC)
  icg_visualizer.c             → numbered TAC with section headers
  icg_pipeline.c               → silent TAC generation, feeds into codegen
        │
        ▼
  [ Code Optimization ]        Peephole optimizations on TAC
  codegen.c                    → algebraic simplification, strength reduction,
                                  constant folding, dead code elimination
        │
        ▼
  [ Target Code Generation ]   Translates optimized TAC to pseudo-assembly
  codegen.c                    → register-based pseudo-assembly (LOAD/STORE/ADD/...)
```
 

 
### Phase 1 — Lexer
 
```
=================== Lexer Output (Token Stream) =====================
 
Token No             Token Type                               Lexeme
-------------------------------------------------------------------------
1                    Keyword                                  int
2                    Identifiers                              x
3                    Separators                               ;
4                    Keyword                                  float
5                    Identifiers                              y
6                    Separators                               ;
7                    Identifiers                              x
8                    Single Character Operators               =
9                    Integers                                 5
...
```
 
Token categories recognized: Keywords, Identifiers, I/O Identifiers, Separators, Integers, Floating point numbers, Arithmetic Operators, Single Character Operators, Multi-character Operators, Logical Operators, Bitwise Operators, Character literals, String literals. Also catches invalid identifiers and unknown characters with line numbers.
 
### Phase 2 — Parser
 
```
1                    Declaration                              int x (line 1)
2                    Declaration                              float y (line 2)
3                    Assignment                               x (line 3)
4                    Assignment                               y (line 4)
5                    If Statement                             6
```
 
### Phase 3 — Semantic Analysis
 
```
========== Symbol Table ==========
 
╔═════╦════════════════╦════════════╦══════════════════════╗
║  #  ║ Name           ║ Type       ║ Line of Declaration  ║
╠═════╬════════════════╬════════════╬══════════════════════╣
║ 1   ║ x              ║ int        ║ 1                    ║
║ 2   ║ y              ║ float      ║ 2                    ║
╚═════╩════════════════╩════════════╩══════════════════════╝
 
========== Semantic Analysis ==========
 
  Checking duplicate declarations...
  OK
 
  Checking undeclared variables...
  OK
 
  Checking type mismatches...
  OK
 
  Checking division by zero...
  OK
 
----------------------------------------
  Status: PASSED — No semantic errors found
========================================
```
 
**On error (from `tests/semantic.txt`):**
```
  [ERROR] Duplicate declaration: 'x' declared again at line 4 (first declared at line 1)
  [ERROR] Undeclared variable: 'z' used but never declared
  [ERROR] Type mismatch: cannot assign 'string' expression to variable 'x' of type 'int'
  [ERROR] Division by zero detected
 
  Status: FAILED — 4 error(s) found
 
ICG skipped due to semantic errors
```
 
### Phase 4 — ICG (Three-Address Code)
 
```
========== Intermediate Code Generation (TAC) ==========
 
-- Assignment --
  1    x = 5
 
-- Assignment --
  2    t1 = x + 3
  3    y = t1
 
-- If Statement --
  4    t2 = x > y
  5    if t2 goto L1
  6    goto L2
  7    L1:
  8    y = x
  9    L2:
 
---------------------------------------------------------
  Total Instructions: 9
=========================================================
```
 
### Phase 5 — Code Optimization
 
```
=== Code Optimization Pass ===
 
[Opt] Simplified: t1 = x + 0  ->  t1 = x
[Opt] Strength Reduction: t2 = y * 2  ->  t2 = y + y
[Opt] Constant Folded: t3 = 5 + 3  ->  t3 = 8
[Opt] Removed Useless Assignment: x = x
 
Applied 4 optimization(s) successfully.
```
 
Optimizations applied: algebraic simplification (`x + 0`, `x * 1`, etc.), strength reduction (`x * 2` → `x + x`), constant folding (`5 + 3` → `8`), dead code elimination (`x = x`).
 
### Phase 6 — Target Code Generation (Pseudo-Assembly)
 
```
=== Final Target Assembly Code ===
 
; x = 5
LOAD R1, 5
STORE x, R1
 
; t1 = x + 3
LOAD R1, x
LOAD R2, 3
ADD R3, R1, R2
STORE t1, R3
 
; if t2 goto L1
LOAD R1, t2
CMP R1, 0
JNE L1
 
; goto L2
JMP L2
 
; L1:
L1:
 
; y = x
LOAD R1, x
STORE y, R1
 
; L2:
L2:
==================================
```
 
---
 
## Build & Run
 
### Prerequisites
 
```bash
sudo apt install flex bison gcc
```

 
### Short Commands (from inside `tests/` folder)
 
```bash
# Lexer Phase
./lexer input4.txt
 
# Parser Phase
./parser input4.txt
 
# Semantic Phase
./semantic input4.txt
 
# ICG Phase
./icg input4.txt
 
# Code Generation Phase
./cdg input4.txt
```
 
---
 
### Incremental Output (from root)
 
Set your input file:
```bash
INPUT=tests/input5.txt
```
 
Then run up to whichever phase you want:
 
```bash
# Till Lexer
./tests/lexer $INPUT
 
# Till Parser
./tests/lexer $INPUT && ./tests/parser $INPUT
 
# Till Semantic
./tests/lexer $INPUT && ./tests/parser $INPUT && ./tests/semantic $INPUT
 
# Till ICG
./tests/lexer $INPUT && ./tests/parser $INPUT && ./tests/semantic $INPUT && ./tests/icg $INPUT
 
# Till Code Generation
./tests/lexer $INPUT && ./tests/parser $INPUT && ./tests/semantic $INPUT && ./tests/icg $INPUT && ./tests/cdg $INPUT
```
 
---
 
## Key Implementation Details
 
| Feature | Detail |
|---|---|
| AST construction | Recursive `create_node(type, value, left, right)` called during parser reductions |
| Shift-reduce resolution | Bison precedence directives (`%left`, `%right`, `%nonassoc`) for all operator classes |
| Dangling-else | `%nonassoc LOWER_THAN_ELSE` + `%nonassoc ELSE` — else binds to nearest if |
| Duplicate detection | O(n²) linked-list scan over declaration chain |
| Undeclared variable check | Recursive AST walk — catches every `id` node against symbol table |
| Type mismatch check | `get_expr_type()` infers expression type, `types_compatible()` allows widening conversions (int→float, int→double, float→double) |
| Division by zero | AST pattern match: `op="/"` with `right=integer("0")` |
| Comment handling | `//` and `/* */` both stripped in Flex using exclusive start condition `%x COMMENT` |
| Compound assignments | Desugared in AST: `x += expr` → `assign(+=, id(x), expr)` — ICG expands to temp + reassign |
| TAC temporaries | `t1, t2, t3, ...` — each expression returns its result temp |
| TAC labels | `L1, L2, L3, ...` — if/if-else/while all generate proper label pairs |
| TAC storage | Silent pipeline stores TAC in a linked list via `add_tac()`, retrieved by codegen via `get_tac_list()` |
| Algebraic simplification | `x + 0`, `x - 0`, `x * 1`, `x / 1` → simplified to just `x` |
| Strength reduction | `x * 2` → `x + x` |
| Constant folding | `5 + 3` → `8` evaluated at compile time |
| Dead code elimination | `x = x` useless assignments removed from TAC list |
| Pseudo-assembly registers | Simplified 3-register model: R1, R2 for operands, R3 for result |
| Assembly instructions | LOAD, STORE, ADD, SUB, MUL, DIV, CMP, JNE, JMP, RET, NEG |
 
---
 
## Supported C Subset
 
| Feature | Supported |
|---|---|
| Data types | `int`, `float`, `double`, `char` |
| Declarations | With and without initialization |
| Arithmetic | `+`, `-`, `*`, `/`, `%`, unary `-` |
| Comparison | `==`, `!=`, `<`, `>`, `<=`, `>=` |
| Logical | `&&`, `\|\|`, `!` |
| Bitwise | `&`, `\|`, `^`, `~` |
| Compound assignment | `+=`, `-=`, `*=`, `/=`, `%=` |
| Increment/Decrement | Pre and post (`++x`, `x++`, `--x`, `x--`) |
| Control flow | `if`, `if-else`, `while` |
| I/O | `printf`, `scanf` |
| Comments | `//` and `/* */` |
| String/char literals | `"..."`, `'...'` |
 
---
 
## Scope

- Implements a defined subset of C (see supported features table above)
- Single-scope, function-level compilation
- Terminal-based output for phase-by-phase visualization
