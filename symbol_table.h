#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdio.h>

typedef struct Variable {
    char *source_name;
    char *storage_name;
    int is_parameter;
    int parameter_index;
    int local_index;
    int scope_level;
    struct Variable *scope_next;
    struct Variable *all_next;
} Variable;

typedef struct FunctionInfo {
    char *name;
    int parameter_count;
    int local_count;
    Variable *variables;
    Variable *variables_tail;
    struct FunctionInfo *next;
} FunctionInfo;

void symbol_table_init(void);
FunctionInfo *symbol_begin_function(const char *name, int line);
void symbol_end_function(void);
char *symbol_add_parameter(const char *name, int line);
char *symbol_declare_variable(const char *name, int line);
char *symbol_new_temporary(void);
char *symbol_resolve_variable(const char *name, int line);
void symbol_enter_scope(void);
void symbol_leave_scope(void);
FunctionInfo *symbol_find_function(const char *name);
FunctionInfo *symbol_current_function(void);
void symbol_print_tables(FILE *output);

#endif
