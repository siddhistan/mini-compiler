# Edu Compiler — C Language Compiler with Phase-by-Phase Visualization

A fully working **C-language compiler** built from scratch using Flex, Bison, and C on Linux/WSL. Covers all four classical compilation phases. Each phase has two implementations: a **standalone visualizer** (prints that phase's output independently) and a **chained version** (feeds into the next phase).

Built as a team project. 9 test programs included in `tests/`.

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
  icg_pipeline.c               → full end-to-end pipeline
```

---

## Project Structure

```
mini-compiler/
├── src/
│   ├── lexer/
│   │   ├── lex_visualizer.l           # Standalone lexer → prints token table
│   │   ├── lexer_parser_visualizer.l  # Lexer chained to parser (returns tokens to Bison)
│   │   └── parser_lexer.l             # Supporting lexer rules for parser phase
│   ├── parser/
│   │   ├── parser_visualizer.y        # Standalone parser → prints construct table
│   │   └── parser_semantic.y          # Parser chained into semantic + ICG
│   ├── semantic/
│   │   ├── semantic_icg.c             # Semantic analysis (4 checks) → calls ICG
│   │   └── symbol_table.c             # Symbol table: type, name, declaration line
│   └── icg/
│       ├── icg_visualizer.c           # TAC generation with numbered instructions
│       └── icg_pipeline.c             # Full end-to-end pipeline
└── tests/
    ├── input1.txt – input8.txt        # 8 C-language test programs
    └── semantic.txt                   # Semantic error test cases
```

---

## Example Output

**Input (`tests/input1.txt`):**
```c
int x;
float y;
x = 5;
y = x + 3;
if (x > y) {
    y = x;
}
```

### Phase 1 — Lexer (standalone)

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

### Phase 2 — Parser (standalone)

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

### Phase 4 — ICG (three-address code)

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

---

## Build & Run

### Prerequisites

```bash
sudo apt install flex bison gcc   # Linux / WSL
```

### Standalone Lexer

```bash
cd src/lexer
flex lex_visualizer.l
gcc lex.yy.c -o lexer -lfl
./lexer ../../tests/input1.txt
```

### Standalone Parser

```bash
cd src/parser
bison -d parser_visualizer.y
cd ../lexer
flex parser_lexer.l
cd ../../
gcc src/parser/parser_visualizer.tab.c src/lexer/lex.yy.c -o parser -lfl
./parser tests/input1.txt
```

### Full Pipeline (Semantic + ICG)

```bash
bison -d src/parser/parser_semantic.y
flex src/lexer/lexer_parser_visualizer.l
gcc src/parser/parser_semantic.tab.c src/lexer/lex.yy.c \
    src/semantic/semantic_icg.c src/icg/icg_visualizer.c \
    src/semantic/symbol_table.c -o compiler -lfl
./compiler tests/input1.txt
```

---

## My Contributions

Team project — two members.

**My files:**
- `src/lexer/lex_visualizer.l` — standalone lexer with full token categorization and formatted output
- `src/lexer/lexer_parser_visualizer.l` — lexer chained into Bison, returns tokens and semantic values (`yylval`)
- `src/parser/parser_visualizer.y` — standalone parser with construct table output, full grammar, precedence rules, dangling-else fix
- `src/parser/parser_semantic.y` — parser that builds the full AST via `create_node()`, chained into semantic + ICG; handles all statement and expression types
- `src/icg/icg_visualizer.c` — full TAC generator: `generate_expr()` for expressions (returns temp names), `generate_statement()` for all statement types, numbered instruction output with section headers

**Teammate's files:**
- `src/semantic/semantic_icg.c` — all four semantic checks (duplicates, undeclared, type mismatch, division by zero), type inference engine, symbol table printer
- `src/semantic/symbol_table.c` — symbol table: linked list of `{type, name, line}` nodes

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

## Current Limitations

- No code optimization phase
- Stops at ICG — no assembly or machine code generation
- No GUI — all output is terminal-based
- Single-scope (no nested function support)
