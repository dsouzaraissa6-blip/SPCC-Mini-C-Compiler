CC = gcc
BISON = bison
FLEX = flex
CFLAGS = -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -O2

COMPILER_SOURCES = main.c parser.tab.c lexer.yy.c symbol_table.c ir.c optimizer.c codegen.c

.PHONY: all clean test

all: compiler

parser.tab.c: parser.y
	$(BISON) -d -o $@ $<

parser.tab.h: parser.tab.c
	@test -f $@

lexer.yy.c: lexer.l parser.tab.h
	$(FLEX) -o $@ $<

compiler: $(COMPILER_SOURCES) compiler.h symbol_table.h ir.h optimizer.h codegen.h
	$(CC) $(CFLAGS) -o $@ $(COMPILER_SOURCES)

test: compiler
	mkdir -p tests/generated
	./compiler tests/arithmetic.c tests/generated/arithmetic.asm tests/generated/arithmetic.txt
	./compiler tests/function_call.c tests/generated/function_call.asm tests/generated/function_call.txt
	./compiler tests/if_else.c tests/generated/if_else.asm tests/generated/if_else.txt
	./compiler tests/while_loop.c tests/generated/while_loop.asm tests/generated/while_loop.txt
	@if ./compiler tests/invalid_syntax.c tests/generated/invalid_syntax.asm tests/generated/invalid_syntax.txt; then \
		echo "ERROR: syntax-error test unexpectedly compiled"; exit 1; \
	else echo "Syntax-error test rejected as expected"; fi
	@if ./compiler tests/invalid_semantic.c tests/generated/invalid_semantic.asm tests/generated/invalid_semantic.txt; then \
		echo "ERROR: semantic-error test unexpectedly compiled"; exit 1; \
	else echo "Semantic-error test rejected as expected"; fi
	@grep -q "add eax, ecx" tests/generated/arithmetic.asm
	@grep -q "call add" tests/generated/function_call.asm
	@grep -q "call sum_down" tests/generated/function_call.asm
	@grep -q "je L" tests/generated/if_else.asm
	@grep -q "jmp L" tests/generated/while_loop.asm
	@grep -q "PHASE 1: LEXICAL ANALYSIS" tests/generated/arithmetic.txt
	@grep -q "VARIABLE / SYMBOL TABLE" tests/generated/arithmetic.txt
	@grep -q "PHASE 6: TARGET CODE GENERATION" tests/generated/arithmetic.txt
	@echo "All compiler tests passed."

clean:
	rm -f compiler parser.tab.c parser.tab.h lexer.yy.c \
		output/output.asm output/output.txt \
		tests/generated/arithmetic.asm tests/generated/arithmetic.txt \
		tests/generated/function_call.asm tests/generated/function_call.txt \
		tests/generated/if_else.asm tests/generated/if_else.txt \
		tests/generated/while_loop.asm tests/generated/while_loop.txt \
		tests/generated/invalid_syntax.asm tests/generated/invalid_syntax.txt \
		tests/generated/invalid_semantic.asm tests/generated/invalid_semantic.txt
