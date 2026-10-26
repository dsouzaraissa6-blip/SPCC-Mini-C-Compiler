%{
#include "compiler.h"
#include "ir.h"
#include "symbol_table.h"

#include <stdlib.h>

int yylex(void);
void yyerror(const char *message);
%}

%code requires {
#include "compiler.h"
}

%define parse.error verbose

%union {
    int integer;
    char *text;
    Value value;
    ArgumentList *arguments;
    BranchLabels *branches;
}

%token INT IF ELSE WHILE RETURN
%token <text> IDENTIFIER
%token <integer> INTEGER
%token LE GE EQ NE AND OR

%type <value> expression initializer
%type <arguments> argument_list argument_list_opt
%type <branches> if_prefix

%left OR
%left AND
%left EQ NE
%left '<' '>' LE GE
%left '+' '-'
%left '*' '/' '%'
%precedence UMINUS '!'
%precedence IF_WITHOUT_ELSE
%precedence ELSE

%start translation_unit

%%

translation_unit:
    function_list
;

function_list:
    function_definition
  | function_list function_definition
;

function_definition:
    INT IDENTIFIER '('
    {
        symbol_begin_function($2, yylineno);
        ir_emit_function_begin($2, yylineno);
        free($2);
    }
    parameter_list_opt ')' function_body
    {
        ir_emit_function_end(yylineno);
        symbol_end_function();
    }
;

parameter_list_opt:
    %empty
  | parameter_list
;

parameter_list:
    parameter
  | parameter_list ',' parameter
;

parameter:
    INT IDENTIFIER
    {
        symbol_add_parameter($2, yylineno);
        free($2);
    }
;

function_body:
    '{' statement_list_opt '}'
;

statement_list_opt:
    %empty
  | statement_list
;

statement_list:
    statement
  | statement_list statement
;

statement:
    declaration
  | assignment_statement
  | expression ';'
  | return_statement
  | selection_statement
  | iteration_statement
  | compound_statement
;

declaration:
    INT declarator_list ';'
;

declarator_list:
    declarator
  | declarator_list ',' declarator
;

declarator:
    IDENTIFIER
    {
        $<text>$ = symbol_declare_variable($1, yylineno);
        free($1);
    }
    initializer
    {
        if ($<text>2 != NULL && $3.place != NULL) {
            ir_emit_move($<text>2, $3, yylineno);
        }
    }
;

initializer:
    %empty
    {
        $$.place = NULL;
    }
  | '=' expression
    {
        $$ = $2;
    }
;

assignment_statement:
    IDENTIFIER '=' expression ';'
    {
        char *destination = symbol_resolve_variable($1, yylineno);
        if (destination != NULL) {
            ir_emit_move(destination, $3, yylineno);
        }
        free($1);
    }
;

return_statement:
    RETURN expression ';'
    {
        ir_emit_return($2, yylineno);
    }
;

selection_statement:
    if_prefix
    statement %prec IF_WITHOUT_ELSE
    {
        ir_emit_label($1->first, yylineno);
        free($1->first);
        free($1->second);
        free($1);
    }
  | if_prefix statement ELSE
    {
        ir_emit_goto($1->second, yylineno);
        ir_emit_label($1->first, yylineno);
    }
    statement
    {
        ir_emit_label($1->second, yylineno);
        free($1->first);
        free($1->second);
        free($1);
    }
;

if_prefix:
    IF '(' expression ')'
    {
        $$ = calloc(1, sizeof(*$$));
        $$->first = ir_new_label();
        $$->second = ir_new_label();
        ir_emit_if_zero($3, $$->first, yylineno);
    }
;

iteration_statement:
    WHILE '('
    {
        BranchLabels *labels = calloc(1, sizeof(*labels));
        labels->first = ir_new_label();
        labels->second = ir_new_label();
        ir_emit_label(labels->first, yylineno);
        $<branches>$ = labels;
    }
    expression ')'
    {
        BranchLabels *labels = $<branches>3;
        ir_emit_if_zero($4, labels->second, yylineno);
    }
    statement
    {
        BranchLabels *labels = $<branches>3;
        ir_emit_goto(labels->first, yylineno);
        ir_emit_label(labels->second, yylineno);
        free(labels->first);
        free(labels->second);
        free(labels);
    }
;

compound_statement:
    '{'
    {
        symbol_enter_scope();
    }
    statement_list_opt '}'
    {
        symbol_leave_scope();
    }
;

expression:
    INTEGER
    {
        $$ = ir_constant($1);
    }
  | IDENTIFIER
    {
        char *place = symbol_resolve_variable($1, yylineno);
        $$.place = place != NULL ? place : ir_constant(0).place;
        free($1);
    }
  | IDENTIFIER '(' argument_list_opt ')'
    {
        $$ = ir_emit_call($1, $3, yylineno);
        argument_list_free($3);
        free($1);
    }
  | '(' expression ')'
    {
        $$ = $2;
    }
  | '-' expression %prec UMINUS
    {
        $$ = ir_emit_unary(IR_NEG, $2, yylineno);
    }
  | '!' expression
    {
        $$ = ir_emit_unary(IR_NOT, $2, yylineno);
    }
  | expression '+' expression
    {
        $$ = ir_emit_binary(IR_ADD, $1, $3, yylineno);
    }
  | expression '-' expression
    {
        $$ = ir_emit_binary(IR_SUB, $1, $3, yylineno);
    }
  | expression '*' expression
    {
        $$ = ir_emit_binary(IR_MUL, $1, $3, yylineno);
    }
  | expression '/' expression
    {
        $$ = ir_emit_binary(IR_DIV, $1, $3, yylineno);
    }
  | expression '%' expression
    {
        $$ = ir_emit_binary(IR_MOD, $1, $3, yylineno);
    }
  | expression '<' expression
    {
        $$ = ir_emit_binary(IR_LT, $1, $3, yylineno);
    }
  | expression '>' expression
    {
        $$ = ir_emit_binary(IR_GT, $1, $3, yylineno);
    }
  | expression LE expression
    {
        $$ = ir_emit_binary(IR_LE, $1, $3, yylineno);
    }
  | expression GE expression
    {
        $$ = ir_emit_binary(IR_GE, $1, $3, yylineno);
    }
  | expression EQ expression
    {
        $$ = ir_emit_binary(IR_EQ, $1, $3, yylineno);
    }
  | expression NE expression
    {
        $$ = ir_emit_binary(IR_NE, $1, $3, yylineno);
    }
  | expression AND expression
    {
        $$ = ir_emit_binary(IR_AND, $1, $3, yylineno);
    }
  | expression OR expression
    {
        $$ = ir_emit_binary(IR_OR, $1, $3, yylineno);
    }
;

argument_list_opt:
    %empty
    {
        $$ = argument_list_new();
    }
  | argument_list
    {
        $$ = $1;
    }
;

argument_list:
    expression
    {
        $$ = argument_list_new();
        argument_list_append($$, $1);
    }
  | argument_list ',' expression
    {
        $$ = $1;
        argument_list_append($$, $3);
    }
;

%%

void yyerror(const char *message)
{
    compiler_error(yylineno, "syntax error: %s", message);
}
