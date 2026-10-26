#include "ir.h"

#include "symbol_table.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

static IRInstruction *first_instruction;
static IRInstruction *last_instruction;
static unsigned int temporary_label;

char *ir_copy_string(const char *text)
{
    char *copy = strdup(text != NULL ? text : "");
    if (copy == NULL) {
        compiler_error(yylineno, "out of memory while creating intermediate code");
    }
    return copy;
}

static IRInstruction *emit(IROp op, const char *result, const char *a,
                           const char *b, int line)
{
    IRInstruction *instruction = calloc(1, sizeof(*instruction));
    if (instruction == NULL) {
        compiler_error(line, "out of memory while creating intermediate code");
        return NULL;
    }
    instruction->op = op;
    instruction->result = result != NULL ? ir_copy_string(result) : NULL;
    instruction->a = a != NULL ? ir_copy_string(a) : NULL;
    instruction->b = b != NULL ? ir_copy_string(b) : NULL;
    instruction->line = line;
    if (last_instruction != NULL) {
        last_instruction->next = instruction;
    } else {
        first_instruction = instruction;
    }
    last_instruction = instruction;
    return instruction;
}

void ir_init(void)
{
    first_instruction = NULL;
    last_instruction = NULL;
    temporary_label = 0;
}

Value ir_constant(int value)
{
    char buffer[48];
    snprintf(buffer, sizeof(buffer), "#%d", value);
    Value result = { ir_copy_string(buffer) };
    return result;
}

static Value new_temporary(void)
{
    Value result = { symbol_new_temporary() };
    return result;
}

Value ir_emit_unary(IROp op, Value operand, int line)
{
    Value result = new_temporary();
    emit(op, result.place, operand.place, NULL, line);
    return result;
}

Value ir_emit_binary(IROp op, Value left, Value right, int line)
{
    Value result = new_temporary();
    emit(op, result.place, left.place, right.place, line);
    return result;
}

ArgumentList *argument_list_new(void)
{
    return calloc(1, sizeof(ArgumentList));
}

void argument_list_append(ArgumentList *list, Value value)
{
    if (list == NULL) {
        return;
    }
    if (list->count == list->capacity) {
        int capacity = list->capacity == 0 ? 4 : list->capacity * 2;
        char **items = realloc(list->items, (size_t)capacity * sizeof(*items));
        if (items == NULL) {
            compiler_error(yylineno, "out of memory while collecting call arguments");
            return;
        }
        list->items = items;
        list->capacity = capacity;
    }
    list->items[list->count++] = ir_copy_string(value.place);
}

void argument_list_free(ArgumentList *list)
{
    if (list == NULL) {
        return;
    }
    for (int index = 0; index < list->count; index++) {
        free(list->items[index]);
    }
    free(list->items);
    free(list);
}

Value ir_emit_call(const char *name, ArgumentList *arguments, int line)
{
    Value result = new_temporary();
    IRInstruction *instruction = emit(IR_CALL, result.place, name, NULL, line);
    if (instruction != NULL && arguments != NULL) {
        instruction->argument_count = arguments->count;
        if (arguments->count > 0) {
            instruction->arguments = calloc((size_t)arguments->count,
                                            sizeof(*instruction->arguments));
            if (instruction->arguments == NULL) {
                compiler_error(line, "out of memory while recording call arguments");
                instruction->argument_count = 0;
            } else {
                for (int index = 0; index < arguments->count; index++) {
                    instruction->arguments[index] = ir_copy_string(arguments->items[index]);
                }
            }
        }
    }
    return result;
}

void ir_emit_move(const char *destination, Value source, int line)
{
    emit(IR_MOVE, destination, source.place, NULL, line);
}

void ir_emit_function_begin(const char *name, int line)
{
    emit(IR_FUNCTION_BEGIN, name, NULL, NULL, line);
}

void ir_emit_function_end(int line)
{
    emit(IR_FUNCTION_END, NULL, NULL, NULL, line);
}

char *ir_new_label(void)
{
    char label[32];
    snprintf(label, sizeof(label), "L%u", temporary_label++);
    return ir_copy_string(label);
}

void ir_emit_label(const char *label, int line)
{
    emit(IR_LABEL, label, NULL, NULL, line);
}

void ir_emit_if_zero(Value condition, const char *label, int line)
{
    emit(IR_IF_ZERO, label, condition.place, NULL, line);
}

void ir_emit_goto(const char *label, int line)
{
    emit(IR_GOTO, label, NULL, NULL, line);
}

void ir_emit_return(Value value, int line)
{
    emit(IR_RETURN, NULL, value.place, NULL, line);
}

void ir_validate_calls(void)
{
    for (IRInstruction *instruction = first_instruction; instruction != NULL;
         instruction = instruction->next) {
        if (instruction->op != IR_CALL) {
            continue;
        }
        FunctionInfo *function = symbol_find_function(instruction->a);
        if (function == NULL) {
            compiler_error(instruction->line, "call to undeclared function '%s'",
                           instruction->a);
        } else if (function->parameter_count != instruction->argument_count) {
            compiler_error(instruction->line,
                           "function '%s' expects %d argument(s), got %d",
                           instruction->a, function->parameter_count,
                           instruction->argument_count);
        }
    }
    if (symbol_find_function("main") == NULL) {
        compiler_error(1, "program must define int main()");
    } else if (symbol_find_function("main")->parameter_count != 0) {
        compiler_error(1, "main must not take parameters");
    }
}

IRInstruction *ir_first_instruction(void)
{
    return first_instruction;
}

static const char *operation_name(IROp op)
{
    switch (op) {
    case IR_FUNCTION_BEGIN: return "function";
    case IR_FUNCTION_END: return "end function";
    case IR_MOVE: return "=";
    case IR_ADD: return "+";
    case IR_SUB: return "-";
    case IR_MUL: return "*";
    case IR_DIV: return "/";
    case IR_MOD: return "%";
    case IR_NEG: return "neg";
    case IR_LT: return "<";
    case IR_GT: return ">";
    case IR_LE: return "<=";
    case IR_GE: return ">=";
    case IR_EQ: return "==";
    case IR_NE: return "!=";
    case IR_AND: return "&&";
    case IR_OR: return "||";
    case IR_NOT: return "!";
    case IR_LABEL: return "label";
    case IR_IF_ZERO: return "ifz";
    case IR_GOTO: return "goto";
    case IR_CALL: return "call";
    case IR_RETURN: return "return";
    }
    return "?";
}

static void print_instruction(FILE *output, const IRInstruction *instruction)
{
    switch (instruction->op) {
    case IR_FUNCTION_BEGIN:
        fprintf(output, "\n%s:\n", instruction->result);
        break;
    case IR_FUNCTION_END:
        fputs("  end function\n", output);
        break;
    case IR_MOVE:
        fprintf(output, "  %s = %s\n", instruction->result, instruction->a);
        break;
    case IR_NEG:
    case IR_NOT:
        fprintf(output, "  %s = %s %s\n", instruction->result,
                operation_name(instruction->op), instruction->a);
        break;
    case IR_ADD: case IR_SUB: case IR_MUL: case IR_DIV: case IR_MOD:
    case IR_LT: case IR_GT: case IR_LE: case IR_GE: case IR_EQ: case IR_NE:
    case IR_AND: case IR_OR:
        fprintf(output, "  %s = %s %s %s\n", instruction->result, instruction->a,
                operation_name(instruction->op), instruction->b);
        break;
    case IR_LABEL:
        fprintf(output, "%s:\n", instruction->result);
        break;
    case IR_IF_ZERO:
        fprintf(output, "  ifz %s goto %s\n", instruction->a, instruction->result);
        break;
    case IR_GOTO:
        fprintf(output, "  goto %s\n", instruction->result);
        break;
    case IR_CALL:
        fprintf(output, "  %s = call %s(", instruction->result, instruction->a);
        for (int index = 0; index < instruction->argument_count; index++) {
            fprintf(output, "%s%s", index == 0 ? "" : ", ",
                    instruction->arguments[index]);
        }
        fputs(")\n", output);
        break;
    case IR_RETURN:
        fprintf(output, "  return %s\n", instruction->a);
        break;
    }
}

void ir_print(FILE *output, const char *heading)
{
    fprintf(output, "%s\n", heading);
    for (IRInstruction *instruction = first_instruction; instruction != NULL;
         instruction = instruction->next) {
        print_instruction(output, instruction);
    }
}
