#ifndef ASM_TREE_H
#define ASM_TREE_H

// gen:enum
typedef enum asm_node_type {
    ASM_NODE_NONE,
    ASM_NODE_PROGRAM,
} asm_node_type;

// TODO: ASM_INSTR_SUB should be a BINOP
// gen:enum
typedef enum asm_instr_type {
    ASM_INSTR_NONE,
    ASM_INSTR_PUSH,
    ASM_INSTR_POP,
    ASM_INSTR_SUB,
    ASM_INSTR_UNOP,
    ASM_INSTR_BINOP,
    ASM_INSTR_IDIV,
    ASM_INSTR_CDQ,
    ASM_INSTR_MOV,
    ASM_INSTR_RET,
} asm_instr_type;

// gen:enum
typedef enum asm_operand_type {
    ASM_OPERAND_NONE,
    ASM_OPERAND_IMM,
    ASM_OPERAND_REG,
    ASM_OPERAND_PSEUDO_REG,
    ASM_OPERAND_STACK,
} asm_operand_type;

// gen:enum
typedef enum asm_unop_type {
    ASM_UNOP_NONE,
    ASM_UNOP_NEG,
    ASM_UNOP_BIT_NEG,
} asm_unop_type;

// gen:enum
typedef enum asm_binop_type {
    ASM_BINOP_NONE,
    ASM_BINOP_ADD,
    ASM_BINOP_SUB,
    ASM_BINOP_MUL,
} asm_binop_type;

typedef enum asm_reg {
    ASM_REG_NONE,
    ASM_REG_RBP,
    ASM_REG_RSP,
    ASM_REG_AX,
    ASM_REG_DX,
    ASM_REG_R10,
    ASM_REG_R11,
    ASM_REG_COUNT,
} asm_reg;

char * reg_strs[] = {
    "(none)",
    "%%rbp",
    "%%rsp",
    "%%eax",
    "%%edx",
    "%%r10d",
    "%%r11d",
};

typedef struct asm_node asm_node;

typedef struct {
    asm_operand_type type;
    union {
        int     value;
        asm_reg reg;
        int     index;
    };
} asm_operand;

typedef struct {
    asm_instr_type type;
    union {
        asm_unop_type  unary_op;
        asm_binop_type binary_op;
    };
    asm_operand src;
    asm_operand dst;
} asm_instr;

def_dyn_arr(asm_instr);
typedef struct {
    char *        name;
    asm_instr_arr instr_arr;
} asm_function;

typedef struct {
    asm_function fn;
} asm_program;

struct asm_node {
    asm_node_type type;
    asm_program   progr;
};

////////////////////////////////////////////////
// Declarations

asm_operand make_imm_asm_operand(int value);
asm_operand make_reg_asm_operand(asm_reg reg);
asm_operand make_pseudo_reg_asm_operand(int index);
asm_operand make_stack_asm_operand(int value);

asm_instr make_asm_push_instr(asm_operand src);
asm_instr make_asm_pop_instr(asm_operand dst);
asm_instr make_asm_sub_instr(asm_operand src, asm_operand dst);
asm_instr make_asm_unop_instr(asm_unop_type op, asm_operand src);
asm_instr make_asm_mov_instr(asm_operand src, asm_operand dst);
asm_instr make_asm_ret_instr(void);

asm_function make_asm_function(char * name);
asm_program  make_asm_program(asm_function fn);
asm_node *   new_asm_program_node(arena * ar, asm_program p);
asm_function asm_function_from_ast(arena * ar, ast * ast_fn);
asm_program  asm_program_from_ast(arena * ar, ast_program ast_p);
asm_node *   asm_tree_from_ast(arena * ar, ast * a);

#endif  // ASM_TREE_H
