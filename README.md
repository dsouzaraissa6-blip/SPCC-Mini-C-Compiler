# Mini C Compiler: Function Calls and Expressions

This is a small C compiler for a teaching subset of C. Its front end is written
with Flex and Bison; the compiler implementation and generated-code backend are
written in C. It creates a phase-by-phase report in an `output` folder,
performs constant folding, and writes 32-bit x86 assembly text there too.

## 1. Build under Ubuntu or WSL

Install the required tools in Ubuntu/WSL:

```sh
sudo apt update
sudo apt install build-essential flex bison
```

Open an Ubuntu terminal in this project directory. If the project is on the
Windows `C:` drive, its WSL path is normally `/mnt/c/Users/raiss/OneDrive/Documents/SPCC/Advanced_Learners_code`.
Then build and compile the sample:

```sh
cd /mnt/c/Users/raiss/OneDrive/Documents/SPCC/Advanced_Learners_code
make
./compiler
```

Edit `input.c`, then run `./compiler`. It reads `input.c` by default and
creates the `output` folder if needed. The full phase-by-phase report is saved
as `output/output.txt`; generated assembly is saved as `output/output.asm`.
The report contains a token table, parsing result, function and variable/symbol
tables, unoptimized and optimized three-address code, and the generated
assembly. The terminal prints both output paths.

Open `output/output.txt` in VS Code to read the report. Each compile replaces
the previous default report and assembly files.

To compile a different source file and choose custom output paths:

```sh
mkdir -p tests/generated
./compiler tests/function_call.c tests/generated/function_call.asm tests/generated/function_call.txt
```

`./compiler` only compiles the current `input.c`; it does not run the test
programs. To optionally run the developer test suite:

```sh
make test
```

Test reports and assembly are kept separately under `tests/generated/`, so
they do not mix with the normal `output/` report. To remove build files and
generated test outputs:

```sh
make clean
```

## 2. Inspecting or executing the assembly

The backend writes GNU assembler-compatible, Intel-syntax 32-bit x86. It uses
the 32-bit System V `cdecl` convention:

* Arguments are pushed from right to left.
* A function saves `EBP`, establishes an `EBP` frame, and puts parameters at
  `[ebp+8]`, `[ebp+12]`, and so on.
* Locals and compiler temporaries occupy 4-byte slots below `EBP`.
* The return value is in `EAX`; the caller removes pushed arguments.
* The callee restores `EBP` and returns with `ret`.

You can inspect `output.asm` directly or assemble it to a 32-bit object:

```sh
as --32 output.asm -o output.o
```

Executing a linked 32-bit program additionally requires 32-bit system
libraries and a compatible linker/runtime. When those are installed, use:

```sh
sudo apt install gcc-multilib libc6-dev-i386
gcc -m32 output.o -o output
./output
```

This small backend emits the `main` function but does not provide C library
functions such as `printf`. The process exit status is the value returned by
`main`.

## 3. Supported language

* `int` functions, integer parameters, local declarations and initializers.
  Multiple declarators such as `int a, b = 2;` are accepted.
* Integer literals, identifiers, assignments, calls, and parentheses.
* `+`, `-`, `*`, `/`, `%`, unary `-`, relational/equality operators, `&&`,
  `||`, and `!`, with C-like precedence and left associativity.
* `if`, `if-else`, `while`, nested blocks, and `return`.
* `//` and `/* ... */` comments.
* Calls to any function defined in the source, including recursive calls.
  Function signatures are checked after parsing, so a function may call a
  function that appears later in the file.

The supported subset intentionally excludes global variables, prototypes,
`void`, arrays, pointers, `for`, `break`, strings, floating-point values,
preprocessor directives, and assignment expressions. Function arguments are
evaluated eagerly from left to right; logical `&&` and `||` also evaluate both
operands (standard C short-circuit behavior is not implemented). Integer
division by zero is not diagnosed at compile time. Every function returns an
`int`; a source function should contain a suitable `return`.

Semantic diagnostics cover undeclared variables, duplicate declarations in the
same scope, duplicate function definitions, calls to undeclared functions,
incorrect call argument counts, and the required zero-parameter `main`.

## 4. Example

`input.c` includes two functions, local variables, a call, an if-else, and a
while loop. Its unoptimized IR includes instructions similar to:

```text
  t0 = p0 + p1
  ifz t1 goto L0
  t2 = v0 * #2
  goto L1
L0:
  ...
L1:
```

The optimized IR folds constant-only arithmetic (for example, `2 + 3 * 4`)
into a constant move. Variables and non-constant expressions remain explicit.
The backend translates IR operations into real instructions such as `add`,
`imul`, `idiv`, `cmp`, `je`, `jmp`, `push`, `call`, and `ret`.

## 5. Test programs and expected diagnostics

The four valid programs in `tests/` cover arithmetic, a function call,
if-else, and a while loop. `input.c` is an additional combined example.

* `tests/invalid_syntax.c`: expected to fail near the missing semicolon, with
  a `syntax error` diagnostic.
* `tests/invalid_semantic.c`: expected to fail with diagnostics for undeclared
  function `missing`, undeclared variable `undeclared_value`, and the wrong
  argument count for `add`.

The compiler exits nonzero on lexical, syntax, semantic, or code-generation
errors and does not report successful compilation.

## 6. Project files and compilation phases

* `lexer.l` — Flex patterns recognize keywords, identifiers, integer literals,
  operators, delimiters, whitespace, and comments. Flex produces `lexer.yy.c`.
* `parser.y` — Bison grammar expresses functions, declarations, statements,
  and precedence. Semantic actions resolve variables and emit IR. Bison
  produces `parser.tab.c` and `parser.tab.h`.
* `symbol_table.h`, `symbol_table.c` — Track functions, parameters, local
  variables, nested block scopes, storage names, and stack slots.
* `ir.h`, `ir.c` — Define and build the three-address IR, labels, branches,
  calls, and returns.
* `optimizer.h`, `optimizer.c` — Fold constant unary/binary arithmetic and
  propagate known temporary constants safely. Operations that would overflow
  or divide by zero are left unchanged.
* `codegen.h`, `codegen.c` — Translate IR into readable, actual x86-32
  assembly with stack frames, arithmetic, comparisons, branches, calls, and
  returns.
* `compiler.h` — Shared compiler data structures and error interface.
* `main.c` — Opens the source, runs parsing and semantic validation, writes
  the phase-by-phase report, and emits assembly only after successful checks.
* `Makefile` — Generates Flex/Bison C sources, builds the compiler, runs
  examples, and cleans generated files while keeping the output folder.
* `input.c` — Combined demonstration program.
* `tests/` — Four valid example programs and invalid syntax/semantic inputs.
  `make test` also writes a separate `.txt` report for each test program in
  `output/`.

For a viva, describe the phases like this:

1. **Lexical analysis:** Flex turns source characters into tokens such as
   `INT`, `IDENTIFIER`, `INTEGER`, and operators.
2. **Syntax analysis:** Bison checks token order against the grammar and
   applies expression precedence; malformed programs produce syntax errors.
3. **Semantic analysis:** The symbol table checks variable scope and duplicate
   declarations. After parsing, recorded calls are checked against all
   function definitions and parameter counts.
4. **Intermediate code generation:** Parser actions create three-address
   instructions, temporaries, and labels independently of the target machine.
5. **Optimization:** A deliberately small constant-folding pass evaluates
   safe constant expressions and substitutes constants for known temporaries.
6. **Target code generation:** The backend maps IR values to stack slots and
   emits 32-bit x86 instructions following the documented `cdecl` convention.
