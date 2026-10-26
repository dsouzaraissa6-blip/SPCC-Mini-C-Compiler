#include "symbol_table.h"

#include "compiler.h"

#include <stdlib.h>
#include <string.h>

typedef struct Scope {
    Variable *variables;
    struct Scope *parent;
    int level;
} Scope;

static FunctionInfo *functions;
static FunctionInfo *functions_tail;
static FunctionInfo *current_function;
static Scope *current_scope;
static int variable_counter;
static int temporary_counter;

static Variable *make_variable(const char *name, int parameter, int parameter_index)
{
    Variable *variable = calloc(1, sizeof(*variable));
    if (variable == NULL) {
        compiler_error(yylineno, "out of memory while creating a variable");
        return NULL;
    }
    variable->source_name = strdup(name);
    variable->is_parameter = parameter;
    variable->parameter_index = parameter_index;
    variable->scope_level = current_scope != NULL ? current_scope->level : 0;

    char storage[64];
    if (parameter) {
        snprintf(storage, sizeof(storage), "p%d", parameter_index);
    } else {
        snprintf(storage, sizeof(storage), "v%d", variable_counter++);
        variable->local_index = current_function->local_count++;
    }
    variable->storage_name = strdup(storage);
    if (variable->source_name == NULL || variable->storage_name == NULL) {
        compiler_error(yylineno, "out of memory while naming a variable");
        free(variable->source_name);
        free(variable->storage_name);
        free(variable);
        return NULL;
    }

    variable->scope_next = current_scope != NULL ? current_scope->variables : NULL;
    if (current_scope != NULL) {
        current_scope->variables = variable;
    }
    if (current_function->variables_tail != NULL) {
        current_function->variables_tail->all_next = variable;
    } else {
        current_function->variables = variable;
    }
    current_function->variables_tail = variable;
    return variable;
}

void symbol_table_init(void)
{
    functions = NULL;
    functions_tail = NULL;
    current_function = NULL;
    current_scope = NULL;
    variable_counter = 0;
    temporary_counter = 0;
}

FunctionInfo *symbol_begin_function(const char *name, int line)
{
    if (symbol_find_function(name) != NULL) {
        compiler_error(line, "duplicate function definition '%s'", name);
    }

    FunctionInfo *function = calloc(1, sizeof(*function));
    Scope *scope = calloc(1, sizeof(*scope));
    if (function == NULL || scope == NULL) {
        compiler_error(line, "out of memory while creating a function");
        free(function);
        free(scope);
        return NULL;
    }
    function->name = strdup(name);
    if (function->name == NULL) {
        compiler_error(line, "out of memory while naming a function");
        free(function);
        free(scope);
        return NULL;
    }
    if (functions_tail != NULL) {
        functions_tail->next = function;
    } else {
        functions = function;
    }
    functions_tail = function;

    current_function = function;
    current_scope = scope;
    return function;
}

void symbol_end_function(void)
{
    while (current_scope != NULL) {
        Scope *parent = current_scope->parent;
        free(current_scope);
        current_scope = parent;
    }
    current_function = NULL;
    temporary_counter = 0;
}

static int name_exists_in_current_scope(const char *name)
{
    if (current_scope == NULL) {
        return 0;
    }
    for (Variable *variable = current_scope->variables; variable != NULL;
         variable = variable->scope_next) {
        if (strcmp(variable->source_name, name) == 0) {
            return 1;
        }
    }
    return 0;
}

char *symbol_add_parameter(const char *name, int line)
{
    if (current_function == NULL || current_scope == NULL) {
        compiler_error(line, "parameter declared outside a function");
        return NULL;
    }
    if (name_exists_in_current_scope(name)) {
        compiler_error(line, "duplicate parameter '%s'", name);
        return NULL;
    }
    Variable *variable = make_variable(name, 1, current_function->parameter_count);
    if (variable == NULL) {
        return NULL;
    }
    current_function->parameter_count++;
    return variable->storage_name;
}

char *symbol_declare_variable(const char *name, int line)
{
    if (current_function == NULL || current_scope == NULL) {
        compiler_error(line, "variable declared outside a function");
        return NULL;
    }
    if (name_exists_in_current_scope(name)) {
        compiler_error(line, "duplicate variable '%s' in this scope", name);
        return NULL;
    }
    Variable *variable = make_variable(name, 0, 0);
    return variable != NULL ? variable->storage_name : NULL;
}

char *symbol_new_temporary(void)
{
    char name[64];
    int number = temporary_counter++;
    snprintf(name, sizeof(name), "temporary%d", number);
    if (current_function == NULL || current_scope == NULL) {
        compiler_error(yylineno, "temporary generated outside a function");
        return strdup("invalid_temp");
    }
    Variable *variable = make_variable(name, 0, 0);
    if (variable == NULL) {
        return strdup("invalid_temp");
    }
    char storage[32];
    snprintf(storage, sizeof(storage), "t%d", number);
    free(variable->storage_name);
    variable->storage_name = strdup(storage);
    if (variable->storage_name == NULL) {
        compiler_error(yylineno, "out of memory while naming a temporary");
        return strdup("invalid_temp");
    }
    return variable->storage_name;
}

char *symbol_resolve_variable(const char *name, int line)
{
    for (Scope *scope = current_scope; scope != NULL; scope = scope->parent) {
        for (Variable *variable = scope->variables; variable != NULL;
             variable = variable->scope_next) {
            if (strcmp(variable->source_name, name) == 0) {
                return variable->storage_name;
            }
        }
    }
    compiler_error(line, "use of undeclared variable '%s'", name);
    return NULL;
}

void symbol_enter_scope(void)
{
    Scope *scope = calloc(1, sizeof(*scope));
    if (scope == NULL) {
        compiler_error(yylineno, "out of memory while entering a scope");
        return;
    }
    scope->parent = current_scope;
    scope->level = current_scope != NULL ? current_scope->level + 1 : 0;
    current_scope = scope;
}

void symbol_leave_scope(void)
{
    if (current_scope == NULL || current_scope->parent == NULL) {
        compiler_error(yylineno, "internal error: cannot leave the function scope");
        return;
    }
    Scope *parent = current_scope->parent;
    free(current_scope);
    current_scope = parent;
}

FunctionInfo *symbol_find_function(const char *name)
{
    for (FunctionInfo *function = functions; function != NULL; function = function->next) {
        if (strcmp(function->name, name) == 0) {
            return function;
        }
    }
    return NULL;
}

FunctionInfo *symbol_current_function(void)
{
    return current_function;
}

void symbol_print_tables(FILE *output)
{
    fputs("\nFUNCTION TABLE\n", output);
    fputs("Function             Parameters  Local slots\n", output);
    fputs("-------------------- ----------  -----------\n", output);
    for (FunctionInfo *function = functions; function != NULL; function = function->next) {
        fprintf(output, "%-20s %-10d %d\n", function->name,
                function->parameter_count, function->local_count);
    }

    fputs("\nVARIABLE / SYMBOL TABLE\n", output);
    fputs("Function             Name                 Kind       Scope  Location\n", output);
    fputs("-------------------- -------------------- ---------- ------ ----------------\n",
          output);
    for (FunctionInfo *function = functions; function != NULL; function = function->next) {
        for (Variable *variable = function->variables; variable != NULL;
             variable = variable->all_next) {
            const char *kind = variable->is_parameter ? "parameter" :
                               variable->storage_name[0] == 't' ? "temporary" : "local";
            char location[32];
            if (variable->is_parameter) {
                snprintf(location, sizeof(location), "EBP+%d",
                         8 + variable->parameter_index * 4);
            } else {
                snprintf(location, sizeof(location), "EBP-%d",
                         4 * (variable->local_index + 1));
            }
            fprintf(output, "%-20s %-20s %-10s %-6d %s\n", function->name,
                    variable->source_name, kind, variable->scope_level, location);
        }
    }
}
