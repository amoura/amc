void asm_operand_codegen_x64(FILE * out, asm_operand oper) {
    switch (oper.type) {
        case ASM_OPERAND_IMM:
            fprintf(out, "$%d", oper.value);
            break;

        case ASM_OPERAND_REG:
            assert(oper.reg < ASM_REG_COUNT);
            fprintf(out, reg_strs[oper.reg]);
            break;

        case ASM_OPERAND_STACK:
            fprintf(out, "-%d(%%rbp)", oper.value);
            break;

        default:
            assert(false);
    }
}

void asm_codegen_x64_instr_1(FILE * out, asm_instr instr, const char * name) {
    fprintf(out, "    %s ", name);
    asm_operand_codegen_x64(out, instr.src);
    fprintf(out, "\n");
}

void asm_codegen_x64_instr_2(FILE * out, asm_instr instr, const char * name) {
    fprintf(out, "    %s ", name);
    asm_operand_codegen_x64(out, instr.src);
    fprintf(out, ", ");
    asm_operand_codegen_x64(out, instr.dst);
    fprintf(out, "\n");
}

void asm_instr_codegen_x64(FILE * out, asm_instr instr) {
    switch (instr.type) {
        case ASM_INSTR_PUSH:
            asm_codegen_x64_instr_1(out, instr, "pushq");
            break;

        case ASM_INSTR_POP:
            asm_codegen_x64_instr_1(out, instr, "popq");
            break;

        case ASM_INSTR_SUB:
            asm_codegen_x64_instr_2(out, instr, "subq");
            break;

        case ASM_INSTR_UNOP:
            switch (instr.op) {
                case OP_NEG:
                    asm_codegen_x64_instr_1(out, instr, "negl");
                    break;
                case OP_BIT_NEG:
                    asm_codegen_x64_instr_1(out, instr, "notl");
                    break;
                default:
                    assert(false);
            }
            break;

        case ASM_INSTR_MOV:
            // TODO: this is a temporary hack
            if (instr.src.type == ASM_OPERAND_REG &&
                instr.src.reg != ASM_REG_R10) {
                asm_codegen_x64_instr_2(out, instr, "movq");
            } else {
                asm_codegen_x64_instr_2(out, instr, "movl");
            }
            break;

        case ASM_INSTR_RET:
            fprintf(out, "    ret\n");
            break;

        default:
            assert(false);
    }
}

void asm_function_codegen_x64(FILE * out, asm_function * fn) {
    assert(fn);
    assert(fn->name);
    fprintf(out, "    .globl %s\n", fn->name);
    fprintf(out, "%s:\n", fn->name);
    for_arr(i, fn->instr_arr) {
        asm_instr_codegen_x64(out, fn->instr_arr.v[i]);
    }
}

void asm_program_codegen_x64(FILE * out, asm_program * p) {
    assert(p);
    asm_function_codegen_x64(out, &p->fn);
    // the below works for linux, not for windows (or mac)
    // fprintf(out, "\n    .section .note.GNU-stack,\"\",@progbits\n");
}

void asm_node_codegen_x64(FILE * out, asm_node * node) {
    // only program node so far
    assert(node);
    assert(node->type == ASM_NODE_PROGRAM);
    asm_program_codegen_x64(out, &node->progr);
}

//////////////////////////////////////////////
// Tests

#ifdef TESTING

void test_codegen_x64_1(void) {
    char * text =
        "int main(void)\n"
        "{\n"
        "    return -(~105);\n"
        "}\n";

    arena     ar = make_arena(Mb(8));
    str_store st = make_str_store(&ar, Mb(2), 10007);
    parser    p  = make_parser(text, "test_codegen_x64", &st, &ar);

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

    // asm_node_codegen_x64(stdout, node);

    free_arena(&ar);
}

#endif
