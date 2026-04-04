#ifndef AMC_IR_H
#define AMC_IR_H

typedef enum {
    IR_VAL_NONE,
    IR_VAL_CONST,
    IR_VAL_VAR,
} ir_val_type;

typedef enum {
    IR_INSTR_NONE,
    IR_INSTR_RETURN,
    IR_INSTR_UNARY,
} ir_instr_type;

typedef struct {
    ir_val_type type;
    union {
        int    int_val;
        char * id;
    };
} ir_val;

typedef struct {
    ir_val val;
} ir_return;

typedef struct {
    op_type op;
    ir_val  src;
    ir_val  dst;
} ir_unary;

typedef struct {
    ir_instr_type type;
    union {
        ir_return ret;
        ir_unary  unary;
    };
} ir_instr;
def_dyn_arr(ir_instr);

typedef struct {
    char *       name;
    ir_instr_arr instrs;
} ir_function;

typedef struct {
    ir_function fn;
} ir_program;

#endif  // AMC_IR_H
