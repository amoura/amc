ir_val make_ir_const(int value) {
    ir_val val  = {0};
    val.type    = IR_VAL_CONST;
    val.int_val = value;
    return val;
}

ir_val make_ir_var(char * id) {
    ir_val val = {0};
    val.type   = IR_VAL_VAR;
    val.id     = id;
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

ir_function make_ir_function(char * name) {
    ir_function fn = {0};
    fn.name        = name;
    return fn;
}

ir_program make_ir_program(ir_function fn) {
    ir_program p = {0};
    p.fn         = fn;
    return p;
}
