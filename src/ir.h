#ifndef AMC_IR_H
#define AMC_IR_H

typedef enum {
    IR_AST_NONE,
    IR_AST_FUNCTION,
    IR_AST_PROGRAM,
} ir_ast_type;

typedef enum {
    IR_VAL_NONE,
    IR_VAL_CONST,
    IR_VAL_VAR,
} ir_val_type;

typedef enum {
    IR_INSTR_NONE,
    IR_INSTR_RETURN,
    IR_INSTR_UNOP,
    IR_INSTR_BINOP,
} ir_instr_type;

typedef struct {
    ir_val_type type;
    union {
        int int_val;
        int var_index;
    };
} ir_val;

typedef struct {
    ir_val val;
} ir_return;

typedef struct {
    unop_type op;
    ir_val    src;
    ir_val    dst;
} ir_unop;

typedef struct {
    binop_type op;
    ir_val     src1;
    ir_val     src2;
    ir_val     dst;
} ir_binop;

typedef struct {
    ir_instr_type type;
    union {
        ir_return ret;
        ir_unop   unop;
        ir_binop  binop;
    };
} ir_instr;
def_dyn_arr(ir_instr);

typedef struct ir_ast ir_ast;

typedef struct {
    char *       name;
    ir_instr_arr instrs;
} ir_function;

typedef struct {
    ir_ast * fn;
} ir_program;

struct ir_ast {
    ir_ast_type type;
    union {
        ir_program  progr;
        ir_function fn;
    };
};

typedef struct {
    int next_index;
} ir_emitter;

#endif  // AMC_IR_H
