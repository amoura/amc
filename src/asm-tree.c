///////////////////////////////////
// Constructors

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
    assert(at_least_one_reg(src, dst));
    assert(asm_is_not_imm(dst));
    return make_asm_instr_2(ASM_INSTR_SUB, src, dst);
}

asm_instr make_asm_unop_instr(op_type op, asm_operand src) {
    asm_instr instr = {0};
    instr.type      = ASM_INSTR_UNOP;
    instr.op        = op;
    instr.src       = src;
    return instr;
}

asm_instr make_asm_mov_instr(asm_operand src, asm_operand dst) {
    assert(at_least_one_reg(src, dst));
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

asm_operand asm_operand_from_ir_val(ir_val val) {
    asm_operand result = {0};
    switch (val.type) {
        case IR_VAL_CONST:
            result = make_imm_asm_operand(val.int_val);
        case IR_VAL_VAR:
            result = make_pseudo_reg_asm_operand(val.var_index);
        default:
            assert(false);
    }
    return result;
}

asm_function asm_function_from_ir(arena * ar, ir_ast * ir_fn) {
    assert(ir_fn);
    assert(ir_fn->type == IR_AST_FUNCTION);
    assert(ir_fn->fn.name);
    asm_function fn = make_asm_function(ir_fn->fn.name);
    for_arr(i, ir_fn->fn.instrs) {
        ir_instr instr = ir_fn->fn.instrs.v[i];
        switch (instr.type) {
            case IR_INSTR_RETURN: {
                asm_operand src = asm_operand_from_ir_val(instr.ret.val);
                asm_operand dst = make_reg_asm_operand(ASM_REG_AX);
                asm_instr_arr_push(ar,
                                   &fn.instr_arr,
                                   make_asm_mov_instr(src, dst));
                asm_instr_arr_push(ar, &fn.instr_arr, make_asm_ret_instr());
            } break;

            case IR_INSTR_UNARY: {
                asm_operand src = asm_operand_from_ir_val(instr.unop.src);
                asm_operand dst = asm_operand_from_ir_val(instr.unop.dst);
                asm_instr_arr_push(ar,
                                   &fn.instr_arr,
                                   make_asm_mov_instr(src, dst));
                asm_instr_arr_push(ar,
                                   &fn.instr_arr,
                                   make_asm_unop_instr(instr.unop.op, dst));
            } break;

            default:
                assert(false);
        }
    }
    return fn;
}

asm_program asm_program_from_ir(arena * ar, ir_program ir_p) {
    assert(ir_p.fn);
    asm_function fn = asm_function_from_ir(ar, ir_p.fn);
    return make_asm_program(fn);
}

asm_node * asm_node_from_ir(arena * ar, ir_ast * ir) {
    assert(ir);
    assert(ir->type == IR_AST_PROGRAM);
    asm_program p = asm_program_from_ir(ar, ir->progr);
    return new_asm_program_node(ar, p);
}

/*
void push_instrs_from_expr_ast(arena *         ar,
                               asm_instr_arr * instr_arr,
                               ast *           expr) {
    assert(expr);
    assert(expr->type == AST_EXPR);
    switch (expr->expr.type) {
        case EXPR_CONST: {
            asm_instr instr =
                make_asm_mov_instr(make_imm_asm_operand(expr->expr.value),
                                   make_reg_asm_operand(ASM_REG_AX));
            asm_instr_arr_push(ar, instr_arr, instr);
            break;
        }

        default:
            assert(false);
    }
}

asm_function asm_function_from_ast(arena * ar, ast * ast_fn) {
    assert(ast_fn);
    assert(ast_fn->type == AST_FUNCTION);
    assert(ast_fn->fn.name);
    assert(ast_fn->fn.body);
    assert(ast_fn->fn.body->type == AST_STMT);
    assert(ast_fn->fn.body->stmt.type == STMT_RETURN);
    assert(ast_fn->fn.body->stmt.ret.expr);
    assert(ast_fn->fn.body->stmt.ret.expr->type == AST_EXPR);
    assert(ast_fn->fn.body->stmt.ret.expr->expr.type == EXPR_CONST);

    asm_function fn   = make_asm_function(ast_fn->fn.name);
    ast *        expr = ast_fn->fn.body->stmt.ret.expr;
    push_instrs_from_expr_ast(ar, &fn.instr_arr, expr);
    asm_instr_arr_push(ar, &fn.instr_arr, make_asm_ret_instr());
    return fn;
}

asm_program asm_program_from_ast(arena * ar, ast_program ast_p) {
    assert(ast_p.fn);
    asm_function fn = asm_function_from_ast(ar, ast_p.fn);
    return make_asm_program(fn);
}

asm_node * asm_tree_from_ast(arena * ar, ast * a) {
    assert(a);
    assert(a->type == AST_PROGRAM);
    asm_program p    = asm_program_from_ast(ar, a->progr);
    asm_node *  node = new_asm_program_node(ar, p);
    return node;
}
*/

///////////////////////////////////////////////////////
// Tests

#ifdef TESTING

void test_asm_tree(void) {
    char * text =
        "int main(void)\n"
        "{\n"
        "    return 105;\n"
        "}\n";

    arena     ar = make_arena(Mb(8));
    str_store st = make_str_store(&ar, Mb(2), 10007);
    parser    p  = make_parser(text, "test_asm_tree_1", &st, &ar);

    ast * a = parse_program(&p);
    assert(a);
    assert(a->type == AST_PROGRAM);
    /*
        asm_node * node = asm_tree_from_ast(&ar, a);
        assert(node);
        assert(node->type == ASM_NODE_PROGRAM);
        assert(node->progr.fn.name);
        assert(node->progr.fn.name == intern_str(&st, "main"));
        assert(node->progr.fn.instr_arr.len == 2);
        assert(node->progr.fn.instr_arr.v[0].type == ASM_INSTR_MOV);
        assert(node->progr.fn.instr_arr.v[1].type == ASM_INSTR_RET);
        assert(node->progr.fn.instr_arr.v[0].src.type == ASM_OPERAND_IMM);
        assert(node->progr.fn.instr_arr.v[0].src.value == 105);
    */
    free_arena(&ar);
}

#endif
