///////////////////////////////////
// Constructors

operand
make_imm_operand(int value)
{
    operand op = {0};
    op.type = OPERAND_IMM;
    op.value = value;
    return op;
}

operand
make_reg_operand(void)
{
    operand op = {0};
    op.type = OPERAND_REG;
    return op;
}

asm_instr
make_asm_mov_instr(operand src, operand dst)
{
    asm_instr instr = {0};
    instr.type = INSTR_MOV;
    instr.src = src;
    instr.dst = dst;
    return instr;
}

asm_instr
make_asm_ret_instr(void)
{
    asm_instr instr = {0};
    instr.type = INSTR_RET;
    return instr;
}

asm_function
make_asm_function(char *name)
{
    asm_function fn = {0};
    fn.name = name;
    return fn;
}

asm_program
make_asm_program(asm_function fn)
{
    asm_program p = {0};
    p.fn = fn;
    return p;
}

asm_node *
new_asm_program_node(arena *ar, asm_program p)
{
    asm_node *node = arena_alloc_type(ar, asm_node);
    return node;
}

//////////////////////////////////////////
// Generating asm_program from Ast

asm_instr *
push_instrs_from_expr_ast(arena *ar, asm_instr *instr_arr, ast *expr)
{
    assert(expr);
    assert(expr->type == AST_EXPR);
    switch (expr->expr.type) {
        case EXPR_CONST: {
            asm_instr instr = make_asm_mov_instr(
                make_reg_operand(),
                make_imm_operand(expr->expr.value)
            );
            arrpush(ar, instr_arr, instr);
            break;
        }

        default:
            assert(false);
    }
    return instr_arr;
}           

asm_function
asm_function_from_ast(arena *ar, ast *ast_fn)
{
    assert(ast_fn);
    assert(ast_fn->type == AST_FUNCTION);
    assert(ast_fn->fn.name);
    assert(ast_fn->fn.body);
    assert(ast_fn->fn.body->type == AST_STMT);
    assert(ast_fn->fn.body->stmt.type == STMT_RETURN);
    assert(ast_fn->fn.body->stmt.ret.expr);
    assert(ast_fn->fn.body->stmt.ret.expr->type == AST_EXPR);
    assert(ast_fn->fn.body->stmt.ret.expr->expr.type == EXPR_CONST);
    int value = ast_fn->fn.body->stmt.ret.expr->expr.value;

    asm_function fn = make_asm_function(ast_fn->fn.name);
    ast *expr = ast_fn->fn.body->stmt.ret.expr;
    fn.instr_arr = push_instrs_from_expr_ast(
            ar,
            fn.instr_arr,
            expr
    );
    arrpush(ar, fn.instr_arr, make_asm_ret_instr());
    return fn;
}

asm_program
asm_program_from_ast(arena *ar, ast_program ast_p)
{
    assert(ast_p.fn);
    asm_function fn = asm_function_from_ast(ar, ast_p.fn);
    return make_asm_program(fn);
}

asm_node *
asm_tree_from_ast(arena *ar, ast *a)
{
    assert(a);
    assert(a->type == AST_PROGRAM);
    asm_program p = asm_program_from_ast(ar, a->progr);
    asm_node *node = new_asm_program_node(ar, p);
    return node;
}



