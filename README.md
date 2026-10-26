# SPCC Mini C Compiler

A mini compiler for a subset of the C programming language, developed as part of the **System Programming and Compiler Construction (SPCC)** course.

The compiler supports function definitions and calls, arithmetic expressions, conditional statements, and loops. It uses **Flex and Bison** for lexical and syntax analysis and is implemented in C.

It generates a phase-by-phase compilation report, performs constant folding, and produces **32-bit x86 assembly code**.

## Table of Contents

* [Features](#features)
* [Technologies Used](#technologies-used)
* [Compilation Phases](#compilation-phases)
* [Getting Started](#getting-started)
* [Usage](#usage)
* [Example Program](#example-program)
* [Supported Language](#supported-language)
* [Testing](#testing)
* [Project Structure](#project-structure)
* [Assembly Generation](#assembly-generation)
* [Limitations](#limitations)

## Features

* **Lexical Analysis:** Recognizes keywords, identifiers, literals, operators, and delimiters.
* **Syntax Analysis:** Validates source code using a Bison grammar.
* **Semantic Analysis:** Checks variable declarations, function definitions, and function call arguments.
* **Intermediate Code Generation:** Generates Three-Address Code (TAC).
* **Code Optimization:** Applies constant folding and safe constant propagation.
* **Target Code Generation:** Produces 32-bit x86 assembly.
* **Error Handling:** Reports lexical, syntax, semantic, and code-generation errors.
* **Compilation Reports:** Saves compilation details and generated assembly in the `output/` directory.

## Technologies Used

| Technology    | Purpose                                 |
| ------------- | --------------------------------------- |
| C             | Compiler implementation                 |
| Flex          | Lexical analysis                        |
| Bison         | Syntax analysis and grammar parsing     |
| GCC           | Compiling the compiler's C source files |
| GNU Assembler | Assembling generated x86 code           |
| Make          | Automating the build process            |
| Ubuntu / WSL  | Development and execution environment   |

## Compilation Phases

The compiler processes source code through the following phases:

1. **Lexical Analysis:** Flex converts source characters into tokens such as keywords, identifiers, integer literals, and operators.
2. **Syntax Analysis:** Bison checks whether the tokens follow the grammar and applies operator precedence.
3. **Semantic Analysis:** The compiler checks variable scope, duplicate declarations, functio
