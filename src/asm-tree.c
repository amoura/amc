///////////////////////////////////
// Constructors

#define asm_reg_rbp make_reg_asm_operand(ASM_REG_RBP)
#define asm_reg_rsp make_reg_asm_operand(ASM_REG_RSP)
#define asm_reg_ax  make_reg_asm_operand(ASM_REG_AX)
#define asm_reg_dx  make_reg_asm_operand(ASM_REG_DX)
#define asm_reg_r10 make_reg_asm_operand(ASM_REG_R10)
#define asm_reg_r11 make_reg_asm_operand(ASM_REG_R11)

asm_unop_type asm_unop_type_from_ir(unop_type ir_op) {
    switch (ir_op) {
        case UNOP_NEG:
            return ASM_UNOP_NEG;
        case UNOP_BIT_NEG:
            return ASM_UNOP_BIT_NEG;
        default:
            assert(false);
    }
    return ASM_UNOP_NONE;
}

asm_binop_type asm_binop_type_from_ir(binop_type ir_op) {
    switch (ir_op) {
        case BINOP_PLUS:
            return ASM_BINOP_ADD;
        case BINOP_MINUS:
            return ASM_BINOP_SUB;
        case BINOP_MUL:
            return ASM_BINOP_MUL;
        default:
            assert(false);
    }
    return ASM_BINOP_NONE;
}

bool asm_is_not_imm(asm_operand a) {
    return a.type != ASM_OPERAND_IMM;
}

bool at_least_one_reg(asm_operand a, asm_operand b) {
    return a.type == ASM_OPERAND_REG || b.type == ASM_OPERAND_REG;
}

asm_operand make_imm_asm_operand(int value) {
    asm_operand op = {0};
    op.type        = ASM_OPERAND_IMM;
    op.value       = value;
    return op;
}

asm_operand make_reg_asm_operand(asm_reg reg) {
    asm_operand op = {0};
    op.type        = ASM_OPERAND_REG;
    op.reg         = reg;
    return op;
}

asm_operand make_pseudo_reg_asm_operand(int index) {
    asm_operand op = {0};
    op.type        = ASM_OPERAND_PSEUDO_REG;
    op.index       = index;
    return op;
}

asm_operand make_stack_asm_operand(int value) {
    asm_operand op = {0};
    op.type        = ASM_OPERAND_STACK;
    op.value       = value;
    return op;
}

asm_instr make_asm_instr_0(asm_instr_type type) {
    asm_instr instr = {0};
    instr.type      = type;
    return instr;
}

asm_instr make_asm_instr_1(asm_instr_type type, asm_operand src) {
    asm_instr instr = {0};
    instr.type      = type;
    instr.src       = src;
    return instr;
}

asm_instr make_asm_instr_2(asm_instr_type type,
                           asm_operand    src,
                           asm_operand    dst) {
    asm_instr instr = {0};
    instr.type      = type;
    instr.src       = src;
    instr.dst       = dst;
    return instr;
}

asm_instr make_asm_push_instr(asm_operand src) {
    return make_asm_instr_1(ASM_INSTR_PUSH, src);
}

asm_instr make_asm_pop_instr(asm_operand dst) {
    return make_asm_instr_1(ASM_INSTR_POP, dst);
}

asm_instr make_asm_sub_instr(asm_operand src, asm_operand dst) {
    assert(asm_is_not_imm(dst));
    return make_asm_instr_2(ASM_INSTR_SUB, src, dst);
}

asm_instr make_asm_unop_instr(asm_unop_type op, asm_operand src) {
    asm_instr instr = {0};
    instr.type      = ASM_INSTR_UNOP;
    instr.unary_op  = op;
    instr.src       = src;
    return instr;
}

asm_instr make_asm_binop_instr(asm_binop_type op,
                               asm_operand    src,
                               asm_operand    dst) {
    asm_instr instr = {0};
    instr.type      = ASM_INSTR_BINOP;
    instr.binary_op = op;
    instr.src       = src;
    instr.dst       = dst;
    return instr;
}

asm_instr make_asm_idiv_instr(asm_operand src) {
    return make_asm_instr_1(ASM_INSTR_IDIV, src);
}

asm_instr make_asm_cdq_instr(void) {
    return make_asm_instr_0(ASM_INSTR_CDQ);
}

asm_instr make_asm_mov_instr(asm_operand src, asm_operand dst) {
    assert(asm_is_not_imm(dst));
    return make_asm_instr_2(ASM_INSTR_MOV, src, dst);
}

asm_instr make_asm_ret_instr(void) {
    return make_asm_instr_0(ASM_INSTR_RET);
}

asm_function make_asm_function(char * name) {
    asm_function fn = {0};
    fn.name         = name;
    return fn;
}

asm_program make_asm_program(asm_function fn) {
    asm_program p = {0};
    p.fn          = fn;
    return p;
}

asm_node * new_asm_program_node(arena * ar, asm_program p) {
    asm_node * node = arena_alloc_type(ar, asm_node);
    node->type      = ASM_NODE_PROGRAM;
    node->progr     = p;
    return node;
}

//////////////////////////////////////////
// Generating asm_program from IR

//////////////////////////////////////////
// First pass: generating the assmebly AST

asm_operand asm_operand_from_ir_val(ir_val val) {
    asm_operand result = {0};
    switch (val.type) {
        case IR_VAL_CONST:
            result = make_imm_asm_operand(val.int_val);
            break;
        case IR_VAL_VAR:
            result = make_pseudo_reg_asm_operand(val.var_index);
            break;
        default:
            assert(false);
    }
    return result;
}

asm_function asm_function_from_ir(arena *      ar,
                                  ir_ast *     ir_fn,
                                  ir_emitter * emitter) {
    assert(ir_fn);
    assert(ir_fn->type == IR_AST_FUNCTION);
    assert(ir_fn->fn.name);
    asm_function fn = make_asm_function(ir_fn->fn.name);

    // Function prologue
    asm_instr_arr_push(ar, &fn.instr_arr, make_asm_push_instr(asm_reg_rbp));
    asm_instr_arr_push(ar,
                       &fn.instr_arr,
                       make_asm_mov_instr(asm_reg_rsp, asm_reg_rbp));
    asm_instr_arr_push(
        ar,
        &fn.instr_arr,
        make_asm_sub_instr(make_imm_asm_operand(4 * emitter->next_index),
                           asm_reg_rsp));

    // Function body
    for_arr(i, ir_fn->fn.instrs) {
        ir_instr instr = ir_fn->fn.instrs.v[i];
        switch (instr.type) {
            case IR_INSTR_RETURN: {
                asm_operand src = asm_operand_from_ir_val(instr.ret.val);
                asm_operand dst = asm_reg_ax;
                asm_instr_arr_push(ar,
                                   &fn.instr_arr,
                                   make_asm_mov_instr(src, dst));
                // Function epilogue
                asm_instr_arr_push(
                    ar,
                    &fn.instr_arr,
                    make_asm_mov_instr(asm_reg_rbp, asm_reg_rsp));
                asm_instr_arr_push(ar,
                                   &fn.instr_arr,
                                   make_asm_pop_instr(asm_reg_rbp));
                asm_instr_arr_push(ar, &fn.instr_arr, make_asm_ret_instr());
            } break;

            case IR_INSTR_UNOP: {
                asm_operand   src = asm_operand_from_ir_val(instr.unop.src);
                asm_operand   dst = asm_operand_from_ir_val(instr.unop.dst);
                asm_unop_type op  = asm_unop_type_from_ir(instr.unop.op);
                asm_instr_arr_push(ar,
                                   &fn.instr_arr,
                                   make_asm_mov_instr(src, dst));
                asm_instr_arr_push(ar,
                                   &fn.instr_arr,
                                   make_asm_unop_instr(op, dst));
            } break;

            case IR_INSTR_BINOP: {
                asm_operand src1 = asm_operand_from_ir_val(instr.binop.src1);
                asm_operand src2 = asm_operand_from_ir_val(instr.binop.src2);
                asm_operand dst  = asm_operand_from_ir_val(instr.binop.dst);

                switch (instr.binop.op) {
                    case BINOP_PLUS:
                    case BINOP_MINUS:
                    case BINOP_MUL: {
                        asm_binop_type op =
                            asm_binop_type_from_ir(instr.binop.op);
                        asm_instr_arr_push(ar,
                                           &fn.instr_arr,
                                           make_asm_mov_instr(src1, dst));
                        asm_instr_arr_push(ar,
                                           &fn.instr_arr,
                                           make_asm_binop_instr(op, src2, dst));
                        break;
                    }

                    case BINOP_REM:
                    case BINOP_DIV: {
                        asm_instr_arr_push(
                            ar,
                            &fn.instr_arr,
                            make_asm_mov_instr(src1, asm_reg_ax));
                        asm_instr_arr_push(ar,
                                           &fn.instr_arr,
                                           make_asm_cdq_instr());
                        asm_instr_arr_push(ar,
                                           &fn.instr_arr,
                                           make_asm_idiv_instr(src2));

                        asm_operand rres = asm_reg_ax;
                        if (instr.binop.op == BINOP_REM) {
                            rres = asm_reg_dx;
                        }
                        asm_instr_arr_push(ar,
                                           &fn.instr_arr,
                                           make_asm_mov_instr(rres, dst));
                        break;
                    }

                    default:
                        assert(false);
                }
                break;
            }

            default:
                assert(false);
        }  // switch (instr.type)
    }  // for_arr(i, ir_fn->fn.instrs)

    return fn;
}

asm_program asm_program_from_ir(arena *      ar,
                                ir_program   ir_p,
                                ir_emitter * emitter) {
    assert(ir_p.fn);
    asm_function fn = asm_function_from_ir(ar, ir_p.fn, emitter);
    return make_asm_program(fn);
}

asm_node * asm_node_from_ir(arena * ar, ir_ast * ir, ir_emitter * emitter) {
    assert(ir);
    assert(ir->type == IR_AST_PROGRAM);
    asm_program p = asm_program_from_ir(ar, ir->progr, emitter);
    return new_asm_program_node(ar, p);
}

///////////////////////////////////////////////////////
// Second pass: replacing pseudo-registers with
//              stack offsets

void asm_operand_replace_pseudo_reg(asm_operand * oper) {
    assert(oper);
    if (oper->type == ASM_OPERAND_PSEUDO_REG) {
        oper->type  = ASM_OPERAND_STACK;
        oper->value = 4 * oper->index;
    }
}

void asm_instr_replace_pseudo_regs(asm_instr * instr) {
    assert(instr);
    switch (instr->type) {
        case ASM_INSTR_SUB:
        case ASM_INSTR_MOV:
        case ASM_INSTR_BINOP:
            asm_operand_replace_pseudo_reg(&instr->src);
            asm_operand_replace_pseudo_reg(&instr->dst);
            break;

        case ASM_INSTR_PUSH:
        case ASM_INSTR_POP:
        case ASM_INSTR_UNOP:
        case ASM_INSTR_IDIV:
            asm_operand_replace_pseudo_reg(&instr->src);
            break;

        default:
            break;
    }
}

void asm_node_replace_pseudo_regs(asm_node * node) {
    assert(node);
    assert(node->type == ASM_NODE_PROGRAM);

    asm_instr_arr * instrs = &node->progr.fn.instr_arr;
    for_arrp(i, instrs) {
        asm_instr * instr = instrs->v + i;
        asm_instr_replace_pseudo_regs(instr);
    }
}

///////////////////////////////////////////////////////
// Third pass

// TODO: this implementation is memory-inefficient, maybe this
// can be improved later...
void asm_node_fix_instrs(arena * ar, asm_node * node) {
    assert(node);
    assert(node->type == ASM_NODE_PROGRAM);

    asm_instr_arr * instrs     = &node->progr.fn.instr_arr;
    asm_instr_arr   new_instrs = {0};
    for_arrp(i, instrs) {
        asm_instr * instr = instrs->v + i;
        if (instr->type == ASM_INSTR_MOV &&
            instr->src.type == ASM_OPERAND_STACK &&
            instr->dst.type == ASM_OPERAND_STACK) {
            asm_instr new_instr1 = *instr;
            new_instr1.dst       = asm_reg_r10;
            asm_instr new_instr2 = *instr;
            new_instr2.src       = asm_reg_r10;
            asm_instr_arr_push(ar, &new_instrs, new_instr1);
            asm_instr_arr_push(ar, &new_instrs, new_instr2);
        } else if (instr->type == ASM_INSTR_BINOP &&
                   (instr->binary_op == ASM_BINOP_ADD ||
                    instr->binary_op == ASM_BINOP_SUB) &&
                   instr->src.type == ASM_OPERAND_STACK &&
                   instr->dst.type == ASM_OPERAND_STACK) {
            asm_instr new_instr1 = make_asm_mov_instr(instr->src, asm_reg_r10);
            asm_instr new_instr2 = *instr;
            new_instr2.src       = asm_reg_r10;
            asm_instr_arr_push(ar, &new_instrs, new_instr1);
            asm_instr_arr_push(ar, &new_instrs, new_instr2);
        } else if (instr->type == ASM_INSTR_BINOP &&
                   instr->binary_op == ASM_BINOP_MUL &&
                   instr->dst.type == ASM_OPERAND_STACK) {
            asm_instr new_instr1 = make_asm_mov_instr(instr->dst, asm_reg_r11);
            asm_instr new_instr2 = *instr;
            new_instr2.dst       = asm_reg_r11;
            asm_instr new_instr3 = make_asm_mov_instr(asm_reg_r11, instr->dst);
            asm_instr_arr_push(ar, &new_instrs, new_instr1);
            asm_instr_arr_push(ar, &new_instrs, new_instr2);
            asm_instr_arr_push(ar, &new_instrs, new_instr3);
        } else if (instr->type == ASM_INSTR_IDIV &&
                   instr->src.type == ASM_OPERAND_IMM) {
            asm_instr new_instr1 = make_asm_mov_instr(instr->src, asm_reg_r10);
            asm_instr new_instr2 = *instr;
            new_instr2.src       = asm_reg_r10;
            asm_instr_arr_push(ar, &new_instrs, new_instr1);
            asm_instr_arr_push(ar, &new_instrs, new_instr2);
        } else {
            asm_instr_arr_push(ar, &new_instrs, *instr);
        }
    }
    // "leaking" memory here
    node->progr.fn.instr_arr = new_instrs;
}

///////////////////////////////////////////////////////
// Combining all passes

asm_node * asm_node_from_ir_all_passes(arena *      ar,
                                       ir_ast *     ir,
                                       ir_emitter * emitter) {
    asm_node * node = asm_node_from_ir(ar, ir, emitter);
    asm_node_replace_pseudo_regs(node);
    asm_node_fix_instrs(ar, node);
    return node;
}

///////////////////////////////////////////////////////
// Tests

#ifdef TESTING

void test_asm_tree(void) {
    char * text =
        "int main(void)\n"
        "{\n"
        "    return ~(-105);\n"
        "}\n";

    arena     ar = make_arena(Mb(8));
    str_store st = make_str_store(&ar, Mb(2), 10007);
    parser    p  = make_parser(text, "test_asm_tree_1", &st, &ar);

    ast * a = parse_program(&p);
    assert(a);
    assert(a->type == AST_PROGRAM);

    ir_emitter emitter = {0};
    ir_ast *   ir      = ir_ast_from_ast(&ar, &emitter, a);
    assert(ir);
    assert(ir->type == IR_AST_PROGRAM);

    asm_node * node = asm_node_from_ir_all_passes(&ar, ir, &emitter);
    assert(node);
    assert(node->type == ASM_NODE_PROGRAM);

    free_arena(&ar);
}

#endif
