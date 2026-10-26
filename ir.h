#ifndef IR_H
#define IR_H

#include "compiler.h"

typedef enum IROp {
    IR_FUNCTION_BEGIN,
    IR_FUNCTION_END,
    IR_MOVE,
    IR_ADD,
    IR_SUB,
    IR_MUL,
    IR_DIV,
    IR_MOD,
    IR_NEG,
    IR_LT,
    IR_GT,
    IR_LE,
    IR_GE,
    IR_EQ,
    IR_NE,
    IR_AND,
    IR_OR,
    IR_NOT,
    IR_LABEL,
    IR_IF_ZERO,
    IR_GOTO,
    IR_CALL,
    IR_RETURN
} IROp;

typedef struct IRInstruction {
    IROp op;
    char *result;
    char *a;
    char *b;
    char **arguments;
    int argument_count;
    int line;
    struct IRInstruction *next;
} IRInstruction;

void ir_init(void);
Value ir_constant(int value);
Value ir_emit_unary(IROp op, Value operand, int line);
Value ir_emit_binary(IROp op, Value left, Value right, int line);
Value ir_emit_call(const char *name, ArgumentList *arguments, int line);
void ir_emit_move(const char *destination, Value source, int line);
void ir_emit_function_begin(const char *name, int line);
void ir_emit_function_end(int line);
char *ir_new_label(void);
void ir_emit_label(const char *label, int line);
void ir_emit_if_zero(Value condition, const char *label, int line);
void ir_emit_goto(const char *label, int line);
void ir_emit_return(Value value, int line);
void ir_validate_calls(void);
void ir_print(FILE *output, const char *heading);
IRInstruction *ir_first_instruction(void);
char *ir_copy_string(const char *text);
ArgumentList *argument_list_new(void);
void argument_list_append(ArgumentList *list, Value value);
void argument_list_free(ArgumentList *list);

#endif
