#include "codegen.h"

#include "compiler.h"
#include "ir.h"
#include "symbol_table.h"

#include <stdlib.h>
#include <string.h>

static Variable *find_variable(FunctionInfo *function, const char *name)
{
    if (function == NULL) {
        return NULL;
    }
    for (Variable *variable = function->variables; variable != NULL;
         variable = variable->all_next) {
        if (strcmp(variable->storage_name, name) == 0) {
            return variable;
        }
    }
    return NULL;
}

static int load_value(FILE *output, FunctionInfo *function, const char *value,
                      const char *register_name, int line)
{
    if (value != NULL && value[0] == '#') {
        fprintf(output, "    mov %s, %s\n", register_name, value + 1);
        return 1;
    }
    Variable *variable = find_variable(function, value);
    if (variable == NULL) {
        compiler_error(line, "code generation cannot find variable '%s'",
                       value != NULL ? value : "(null)");
        return 0;
    }
    if (variable->is_parameter) {
        fprintf(output, "    mov %s, DWORD PTR [ebp+%d]\n", register_name,
                8 + variable->parameter_index * 4);
    } else {
        fprintf(output, "    mov %s, DWORD PTR [ebp-%d]\n", register_name,
                4 * (variable->local_index + 1));
    }
    return 1;
}

static int store_value(FILE *output, FunctionInfo *function, const char *destination,
                       const char *register_name, int line)
{
    Variable *variable = find_variable(function, destination);
    if (variable == NULL || variable->is_parameter) {
        compiler_error(line, "code generation cannot store variable '%s'",
                       destination != NULL ? destination : "(null)");
        return 0;
    }
    fprintf(output, "    mov DWORD PTR [ebp-%d], %s\n",
            4 * (variable->local_index + 1), register_name);
    return 1;
}

static const char *comparison_instruction(IROp op)
{
    switch (op) {
    case IR_LT: return "setl";
    case IR_GT: return "setg";
    case IR_LE: return "setle";
    case IR_GE: return "setge";
    case IR_EQ: return "sete";
    case IR_NE: return "setne";
    default: return NULL;
    }
}

static int emit_instruction(FILE *output, FunctionInfo *function,
                            const IRInstruction *instruction)
{
    int ok = 1;
    switch (instruction->op) {
    case IR_FUNCTION_BEGIN:
        fprintf(output, "\n    .globl %s\n    .type %s, @function\n%s:\n",
                instruction->result, instruction->result, instruction->result);
        fputs("    push ebp\n    mov ebp, esp\n", output);
        if (function->local_count > 0) {
            fprintf(output, "    sub esp, %d\n", function->local_count * 4);
        }
        break;
    case IR_FUNCTION_END:
        fprintf(output, ".Lreturn_%s:\n    mov esp, ebp\n    pop ebp\n    ret\n",
                function->name);
        fprintf(output, "    .size %s, .-%s\n", function->name, function->name);
        break;
    case IR_MOVE:
        ok &= load_value(output, function, instruction->a, "eax", instruction->line);
        ok &= store_value(output, function, instruction->result, "eax",
                          instruction->line);
        break;
    case IR_ADD:
    case IR_SUB:
    case IR_MUL:
    case IR_DIV:
    case IR_MOD:
    case IR_LT:
    case IR_GT:
    case IR_LE:
    case IR_GE:
    case IR_EQ:
    case IR_NE:
    case IR_AND:
    case IR_OR:
        ok &= load_value(output, function, instruction->a, "eax", instruction->line);
        ok &= load_value(output, function, instruction->b, "ecx", instruction->line);
        switch (instruction->op) {
        case IR_ADD: fputs("    add eax, ecx\n", output); break;
        case IR_SUB: fputs("    sub eax, ecx\n", output); break;
        case IR_MUL: fputs("    imul eax, ecx\n", output); break;
        case IR_DIV:
        case IR_MOD:
            fputs("    cdq\n    idiv ecx\n", output);
            if (instruction->op == IR_MOD) {
                fputs("    mov eax, edx\n", output);
            }
            break;
        case IR_AND:
            fputs("    test eax, eax\n    setne al\n    movzx eax, al\n"
                  "    test ecx, ecx\n    setne cl\n    movzx ecx, cl\n"
                  "    and eax, ecx\n", output);
            break;
        case IR_OR:
            fputs("    test eax, eax\n    setne al\n    movzx eax, al\n"
                  "    test ecx, ecx\n    setne cl\n    movzx ecx, cl\n"
                  "    or eax, ecx\n", output);
            break;
        default:
            fprintf(output, "    cmp eax, ecx\n    %s al\n    movzx eax, al\n",
                    comparison_instruction(instruction->op));
            break;
        }
        ok &= store_value(output, function, instruction->result, "eax",
                          instruction->line);
        break;
    case IR_NEG:
        ok &= load_value(output, function, instruction->a, "eax", instruction->line);
        fputs("    neg eax\n", output);
        ok &= store_value(output, function, instruction->result, "eax",
                          instruction->line);
        break;
    case IR_NOT:
        ok &= load_value(output, function, instruction->a, "eax", instruction->line);
        fputs("    cmp eax, 0\n    sete al\n    movzx eax, al\n", output);
        ok &= store_value(output, function, instruction->result, "eax",
                          instruction->line);
        break;
    case IR_LABEL:
        fprintf(output, "%s:\n", instruction->result);
        break;
    case IR_IF_ZERO:
        ok &= load_value(output, function, instruction->a, "eax", instruction->line);
        fprintf(output, "    cmp eax, 0\n    je %s\n", instruction->result);
        break;
    case IR_GOTO:
        fprintf(output, "    jmp %s\n", instruction->result);
        break;
    case IR_CALL:
        for (int index = instruction->argument_count - 1; index >= 0; index--) {
            ok &= load_value(output, function, instruction->arguments[index],
                             "eax", instruction->line);
            fputs("    push eax\n", output);
        }
        fprintf(output, "    call %s\n", instruction->a);
        if (instruction->argument_count > 0) {
            fprintf(output, "    add esp, %d\n", instruction->argument_count * 4);
        }
        ok &= store_value(output, function, instruction->result, "eax",
                          instruction->line);
        break;
    case IR_RETURN:
        ok &= load_value(output, function, instruction->a, "eax", instruction->line);
        fprintf(output, "    jmp .Lreturn_%s\n", function->name);
        break;
    }
    return ok;
}

int generate_assembly(FILE *output)
{
    fputs(".intel_syntax noprefix\n    .text\n", output);
    FunctionInfo *function = NULL;
    int success = 1;
    for (IRInstruction *instruction = ir_first_instruction(); instruction != NULL;
         instruction = instruction->next) {
        if (instruction->op == IR_FUNCTION_BEGIN) {
            function = symbol_find_function(instruction->result);
            if (function == NULL) {
                compiler_error(instruction->line,
                               "missing function information for '%s'",
                               instruction->result);
                return 0;
            }
        }
        if (function == NULL) {
            compiler_error(instruction->line,
                           "internal error: instruction outside a function");
            return 0;
        }
        success &= emit_instruction(output, function, instruction);
        if (instruction->op == IR_FUNCTION_END) {
            function = NULL;
        }
    }
    fputs("\n    .section .note.GNU-stack,\"\",@progbits\n", output);
    return success;
}
