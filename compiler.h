#ifndef COMPILER_H
#define COMPILER_H

#include <stdio.h>

typedef struct Value {
    char *place;
} Value;

typedef struct ArgumentList {
    char **items;
    int count;
    int capacity;
} ArgumentList;

typedef struct BranchLabels {
    char *first;
    char *second;
} BranchLabels;

extern int compiler_errors;
extern FILE *yyin;
extern FILE *compiler_report;
extern int yylineno;

void compiler_error(int line, const char *format, ...);
void compiler_record_token(const char *category, const char *lexeme, int line);

#endif
