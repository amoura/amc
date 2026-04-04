ir_val make_ir_const(int value) {
    ir_val val  = {0};
    val.type    = IR_VAL_CONST;
    val.int_val = value;
    return val;
}

ir_val make_ir_var(int index) {
    ir_val val    = {0};
    val.type      = IR_VAL_VAR;
    val.var_index = index;
    return val;
}

ir_instr make_ir_return_instr(ir_val val) {
    ir_instr instr = {0};
    instr.type     = IR_INSTR_RETURN;
    instr.ret.val  = val;
    return instr;
}

ir_instr make_ir_unary_instr(op_type op, ir_val src, ir_val dst) {
    assert(is_unary_op(op));
    ir_instr instr  = {0};
    instr.type      = IR_INSTR_UNARY;
    instr.unary.op  = op;
    instr.unary.src = src;
    instr.unary.dst = dst;
    return instr;
}

ir_ast * new_ir_function(arena * ar, char * name) {
    ir_ast * fn = arena_alloc_type(ar, ir_ast);
    fn->type    = IR_AST_FUNCTION;
    fn->fn.name = name;
    zero_struct(&fn->fn.instrs);
    return fn;
}

ir_ast * new_ir_program(arena * ar, ir_ast * fn) {
    ir_ast * p  = arena_alloc_type(ar, ir_ast);
    p->type     = IR_AST_PROGRAM;
    p->progr.fn = fn;
    return p;
}

////////////////////////////////////////////////////
// Emit IR from AST

ir_val get_new_ir_var(ir_emitter * emitter) {
    return make_ir_var(emitter->next_index++);
}

ir_val push_ir_instrs_from_expr(arena *        ar,
                                ir_emitter *   emitter,
                                ast *          expr,
                                ir_instr_arr * arr) {
    assert(expr);
    assert(expr->type == AST_EXPR);

    switch (expr->expr.type) {
        case EXPR_CONST:
            return make_ir_const(expr->expr.value);

        case EXPR_UNOP: {
            ir_val   src   = push_ir_instrs_from_expr(ar,
                                                      emitter,
                                                      expr->expr.unop.expr,
                                                      arr);
            ir_val   dst   = get_new_ir_var(emitter);
            ir_instr unary = make_ir_unary_instr(expr->expr.unop.op, src, dst);
            ir_instr_arr_push(ar, arr, unary);
            return dst;
        }

        default:
            assert(false);
            return (ir_val){0};
    }
}

void push_ir_instrs_from_stmt(arena *        ar,
                              ir_emitter *   emitter,
                              ast *          stmt,
                              ir_instr_arr * arr) {
    assert(stmt);
    assert(stmt->type == AST_STMT);

    switch (stmt->stmt.type) {
        case STMT_RETURN: {
            assert(stmt->stmt.ret.expr);
            ir_val val_last_instr =
                push_ir_instrs_from_expr(ar, emitter, stmt->stmt.ret.expr, arr);
            ir_instr ret = make_ir_return_instr(val_last_instr);
            ir_instr_arr_push(ar, arr, ret);
            break;
        }
        default:
            assert(false);
    }
}

ir_ast * ir_ast_from_ast(arena * ar, ir_emitter * emitter, ast * a) {
    assert(ar);
    assert(a);

    switch (a->type) {
        case AST_PROGRAM:
            return new_ir_program(ar,
                                  ir_ast_from_ast(ar, emitter, a->progr.fn));

        case AST_FUNCTION: {
            ir_ast * ir_fn = new_ir_function(ar, a->fn.name);
            push_ir_instrs_from_stmt(ar,
                                     emitter,
                                     a->fn.body,
                                     &ir_fn->fn.instrs);
            return ir_fn;
        }

        default:
            assert(false);
    }
    return NULL;
}
