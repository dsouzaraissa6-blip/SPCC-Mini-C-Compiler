%option noyywrap yylineno noinput nounput

%{
#include "parser.tab.h"
#include "compiler.h"

#include <stdlib.h>
#include <string.h>

static char *copy_lexeme(const char *text, size_t length)
{
    char *copy = malloc(length + 1);
    if (copy == NULL) {
        compiler_error(yylineno, "out of memory while reading an identifier");
        return NULL;
    }
    memcpy(copy, text, length);
    copy[length] = '\0';
    return copy;
}

#define EMIT_TOKEN(token, category) \
    do { \
        compiler_record_token(category, yytext, yylineno); \
        return token; \
    } while (0)
%}

DIGIT       [0-9]
IDENT       [a-zA-Z_][a-zA-Z0-9_]*

%%

"int"                       { EMIT_TOKEN(INT, "keyword"); }
"if"                        { EMIT_TOKEN(IF, "keyword"); }
"else"                      { EMIT_TOKEN(ELSE, "keyword"); }
"while"                     { EMIT_TOKEN(WHILE, "keyword"); }
"return"                    { EMIT_TOKEN(RETURN, "keyword"); }
"<="                        { EMIT_TOKEN(LE, "relational operator"); }
">="                        { EMIT_TOKEN(GE, "relational operator"); }
"=="                        { EMIT_TOKEN(EQ, "equality operator"); }
"!="                        { EMIT_TOKEN(NE, "equality operator"); }
"&&"                        { EMIT_TOKEN(AND, "logical operator"); }
"||"                        { EMIT_TOKEN(OR, "logical operator"); }
"//"[^\n]*                  { }
"/*"([^*]|\*+[^*/])*\*+"/"  { }
[ \t\r\n]+                  { }
{DIGIT}+                    {
                                long long value = strtoll(yytext, NULL, 10);
                                if (value > 2147483647LL) {
                                    compiler_error(yylineno,
                                                   "integer literal is too large");
                                    yylval.integer = 0;
                                } else {
                                    yylval.integer = (int)value;
                                }
                                EMIT_TOKEN(INTEGER, "integer constant");
                            }
{IDENT}                     {
                                yylval.text = copy_lexeme(yytext, (size_t)yyleng);
                                EMIT_TOKEN(IDENTIFIER, "identifier");
                            }
[{}(),;]                     { EMIT_TOKEN(yytext[0], "delimiter"); }
[=+\-*/%<>!]                 { EMIT_TOKEN(yytext[0], "operator"); }
.                           {
                                compiler_error(yylineno, "unexpected character '%s'",
                                               yytext);
                                return yytext[0];
                            }

%%
