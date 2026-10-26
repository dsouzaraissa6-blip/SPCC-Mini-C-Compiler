#include "optimizer.h"

#include "ir.h"

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct KnownConstant {
    char *name;
    char *value;
    struct KnownConstant *next;
} KnownConstant;

static int parse_constant(const char *text, int *value)
{
    if (text == NULL || text[0] != '#') {
        return 0;
    }
    char *end;
    long parsed = strtol(text + 1, &end, 10);
    if (*end != '\0' || parsed < INT_MIN || parsed > INT_MAX) {
        return 0;
    }
    *value = (int)parsed;
    return 1;
}

static const char *known_value(KnownConstant *constants, const char *name)
{
    for (KnownConstant *item = constants; item != NULL; item = item->next) {
        if (strcmp(item->name, name) == 0) {
            return item->value;
        }
    }
    return NULL;
}

static void remember_constant(KnownConstant **constants, const char *name,
                              const char *value)
{
    if (name == NULL || name[0] != 't') {
        return;
    }
    KnownConstant *item = malloc(sizeof(*item));
    if (item == NULL) {
        return;
    }
    item->name = ir_copy_string(name);
    item->value = ir_copy_string(value);
    item->next = *constants;
    *constants = item;
}

static void replace_known_operand(KnownConstant *constants, char **operand)
{
    const char *value = *operand != NULL ? known_value(constants, *operand) : NULL;
    if (value != NULL) {
        *operand = ir_copy_string(value);
    }
}

void optimize_ir(void)
{
    KnownConstant *constants = NULL;
    for (IRInstruction *instruction = ir_first_instruction(); instruction != NULL;
         instruction = instruction->next) {
        if (instruction->op == IR_FUNCTION_BEGIN) {
            constants = NULL;
            continue;
        }
        replace_known_operand(constants, &instruction->a);
        replace_known_operand(constants, &instruction->b);
        for (int index = 0; index < instruction->argument_count; index++) {
            replace_known_operand(constants, &instruction->arguments[index]);
        }

        int left;
        int right;
        int result;
        if (instruction->op == IR_NEG || instruction->op == IR_NOT) {
            if (!parse_constant(instruction->a, &left)) {
                continue;
            }
            if (instruction->op == IR_NEG) {
                if (left == INT_MIN) {
                    continue;
                }
                result = -left;
            } else {
                result = !left;
            }
            instruction->op = IR_MOVE;
            char constant[48];
            snprintf(constant, sizeof(constant), "#%d", result);
            instruction->a = ir_copy_string(constant);
            instruction->b = NULL;
            remember_constant(&constants, instruction->result, constant);
            continue;
        }
        if (instruction->op < IR_ADD || instruction->op > IR_OR ||
            !parse_constant(instruction->a, &left) ||
            !parse_constant(instruction->b, &right)) {
            continue;
        }
        int64_t wide_result;
        switch (instruction->op) {
        case IR_ADD: wide_result = (int64_t)left + right; break;
        case IR_SUB: wide_result = (int64_t)left - right; break;
        case IR_MUL: wide_result = (int64_t)left * right; break;
        case IR_DIV:
            if (right == 0 || (left == INT_MIN && right == -1)) continue;
            wide_result = left / right;
            break;
        case IR_MOD:
            if (right == 0 || (left == INT_MIN && right == -1)) continue;
            wide_result = left % right;
            break;
        case IR_LT: wide_result = left < right; break;
        case IR_GT: wide_result = left > right; break;
        case IR_LE: wide_result = left <= right; break;
        case IR_GE: wide_result = left >= right; break;
        case IR_EQ: wide_result = left == right; break;
        case IR_NE: wide_result = left != right; break;
        case IR_AND: wide_result = (left != 0) && (right != 0); break;
        case IR_OR: wide_result = (left != 0) || (right != 0); break;
        default: continue;
        }
        if (wide_result < INT_MIN || wide_result > INT_MAX) {
            continue;
        }
        result = (int)wide_result;
        instruction->op = IR_MOVE;
        char constant[48];
        snprintf(constant, sizeof(constant), "#%d", result);
        instruction->a = ir_copy_string(constant);
        instruction->b = NULL;
        remember_constant(&constants, instruction->result, constant);
    }
}
