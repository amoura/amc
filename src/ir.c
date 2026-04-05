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

ir_instr make_ir_unop_instr(op_type op, ir_val src, ir_val dst) {
    assert(is_unop(op));
    ir_instr instr = {0};
    instr.type     = IR_INSTR_UNARY;
    instr.unop.op  = op;
    instr.unop.src = src;
    instr.unop.dst = dst;
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
            ir_val   src  = push_ir_instrs_from_expr(ar,
                                                     emitter,
                                                     expr->expr.unop.expr,
                                                     arr);
            ir_val   dst  = get_new_ir_var(emitter);
            ir_instr unop = make_ir_unop_instr(expr->expr.unop.op, src, dst);
            ir_instr_arr_push(ar, arr, unop);
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

/////////////////////////////////////////////////////////////
// Pretty-printing

void print_ir_val(FILE * stream, ir_val val) {
    switch (val.type) {
        case IR_VAL_CONST:
            fprintf(stream, "Const(%d)", val.int_val);
            break;

        case IR_VAL_VAR:
            fprintf(stream, "Var(\"tmp.%d\")", val.var_index);
            break;

        default:
            assert(false);
    }
}

void print_ir_instr(FILE * stream, ir_instr instr, int indent) {
    print_indent(stream, indent + 4);
    switch (instr.type) {
        case IR_INSTR_RETURN:
            fprintf(stream, "Return(");
            print_ir_val(stream, instr.ret.val);
            fprintf(stream, ")\n");
            break;

        case IR_INSTR_UNARY:
            fprintf(stream, "Unary(%s, ", op_type_to_str(instr.unop.op));
            print_ir_val(stream, instr.unop.src);
            fprintf(stream, ", ");
            print_ir_val(stream, instr.unop.dst);
            fprintf(stream, ")\n");
            break;

        default:
            assert(false);
    }
}

void print_ir_ast(FILE * stream, ir_ast * ir, int indent) {
    assert(ir);
    print_indent(stream, indent);
    switch (ir->type) {
        case IR_AST_PROGRAM:
            fprintf(stream, "Program:\n");
            print_ir_ast(stream, ir->progr.fn, indent + 4);
            break;

        case IR_AST_FUNCTION:
            fprintf(stream, "Function:\n");
            print_indent(stream, indent + 4);
            fprintf(stream, "name: %s\n", ir->fn.name);
            print_indent(stream, indent + 4);
            fprintf(stream, "body:\n");
            for_arr(i, ir->fn.instrs) {
                print_ir_instr(stream, ir->fn.instrs.v[i], indent + 6);
            }
            break;

        default:
            assert(false);
    }
}

/////////////////////////////////////////////////////////////
// Tests

#ifdef TESTING

void test_ir_basic() {
    char * text =
        "int main(void)\n"
        "{\n"
        "    return ~(-105);\n"
        "}\n";

    arena     ar = make_arena(Mb(8));
    str_store st = make_str_store(&ar, Mb(2), 10007);
    parser    p  = make_parser(text, "test_parser_1_basic", &st, &ar);

    ast * a = parse_program(&p);
    assert(a);
    assert(a->type == AST_PROGRAM);

    ir_emitter emitter = {0};

    ir_ast * ir = ir_ast_from_ast(&ar, &emitter, a);
    assert(ir);
    assert(ir->type == IR_AST_PROGRAM);
    assert(ir->progr.fn);
    assert(ir->progr.fn->type == IR_AST_FUNCTION);
    assert(ir->progr.fn->fn.name == intern_str(&st, "main"));
    assert(ir->progr.fn->fn.instrs.len > 0);

    // print_ir_ast(stdout, ir, 0);

    free_arena(&ar);
}

#endif
