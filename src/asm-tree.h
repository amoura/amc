#ifndef ASM_TREE_H
#define ASM_TREE_H

//gen:enum
typedef enum asm_node_type {
    ASM_NODE_NONE,
    ASM_NODE_PROGRAM,
} asm_node_type;

//gen:enum
typedef enum instr_type {
    INSTR_NONE,
    INSTR_MOV,
    INSTR_RET,
} instr_type;

//gen:enum
typedef enum operand_type {
    OPERAND_NONE,
    OPERAND_IMM,
    OPERAND_REG,
} operand_type;

typedef struct asm_node asm_node;

typedef struct {
    operand_type type;
    int value;
} operand;

typedef struct {
    instr_type type;
    operand src;
    operand dst;
} asm_instr;

def_dyn_arr(asm_instr);
typedef struct {
    char *name;
    asm_instr_arr instr_arr; 
} asm_function;

typedef struct {
    asm_function fn;
} asm_program;

struct asm_node {
    asm_node_type type;
    asm_program progr;
};
    

#endif // ASM_TREE_H
