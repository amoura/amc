void asm_operand_codegen_x64(FILE * out, asm_operand oper) {
    switch (oper.type) {
        case ASM_OPERAND_IMM:
            fprintf(out, "$%d", oper.value);
            break;

        case ASM_OPERAND_REG:
            fprintf(out, "%%eax");
            break;

        default:
            assert(false);
    }
}

void asm_instr_codegen_x64(FILE * out, asm_instr instr) {
    switch (instr.type) {
        case ASM_INSTR_MOV:
            fprintf(out, "    movl ");
            asm_operand_codegen_x64(out, instr.src);
            fprintf(out, ", ");
            asm_operand_codegen_x64(out, instr.dst);
            fprintf(out, "\n");
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
        "    return 105;\n"
        "}\n";

    arena     ar = make_arena(Mb(8));
    str_store st = make_str_store(&ar, Mb(2), 10007);
    parser    p  = make_parser(text, "test_codegen_x64", &st, &ar);

    ast * a = parse_program(&p);
    assert(a);
    assert(a->type == AST_PROGRAM);

    asm_node * node = asm_tree_from_ast(&ar, a);
    assert(node);
    assert(node->type == ASM_NODE_PROGRAM);

    // asm_node_codegen_x64(stdout, node);
}

#endif
