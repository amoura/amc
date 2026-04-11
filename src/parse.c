parser make_parser(char * text, char * source, str_store * st, arena * ar) {
    lexer  lex = make_lexer(text, source, strlen(text), st);
    parser p   = {0};
    p.lex      = lex;
    p.ar       = ar;
    return p;
}

void print_parse_error(FILE * stream, parser * p, ast * a) {
    assert(stream);
    assert(p);

    text_pos pos = curr_text_pos(&p->lex);
    print_error_location(stream, p->lex.text, pos.line - 3, pos.line, pos.col);
    for (int depth = 0; a; a = a->err.next, depth++) {
        assert(a->type == AST_ERR);

        char * msg;
        if (depth == 0) {
            msg = "parsing error";
        } else {
            msg = ".............";
        }

        fprintf(stream,
                "%s:%d:%d: %s (%d): ",
                p->lex.source,
                pos.line,
                pos.col,
                msg,
                depth);
        switch (a->err.type) {
            case AST_ERR_EXP_TOKEN:
                fprintf(stream,
                        "expected token %s, found %s\n",
                        token_type_to_msg(a->err.exp_token_type),
                        token_type_to_msg(a->err.found_tok_type));
                break;

            case AST_ERR_EXP_AST:
                fprintf(stream,
                        "expected %s\n",
                        ast_type_to_msg(a->err.exp_ast_type));
                break;

            default:
                assert(false);
        }
    }
}

#define expect_ast(a, p, ast_type)                                             \
    do {                                                                       \
        if (ast_is_err(a)) {                                                   \
            return new_ast_error_exp_ast(p->ar, a, ast_type);                  \
        }                                                                      \
    } while (false)

#define expect_token(tok, p, exp_token_type)                                   \
    do {                                                                       \
        if (tok.type != exp_token_type) {                                      \
            return new_ast_error_exp_token(p->ar, exp_token_type, tok.type);   \
        }                                                                      \
    } while (false)

#define expect_and_consume_token(p, exp_token_type)                            \
    do {                                                                       \
        token tok_ = next_token(&p->lex);                                      \
        if (tok_.type != exp_token_type) {                                     \
            return new_ast_error_exp_token(p->ar, exp_token_type, tok_.type);  \
        }                                                                      \
    } while (false)

void consume_token(parser * p) {
    next_token(&p->lex);
}

/////////////////////////////////////////////////////////
// Recursive descent

ast * parse_program(parser * p) {
    ast * fn = parse_function(p);
    expect_ast(fn, p, AST_FUNCTION);
    expect_and_consume_token(p, TOK_EOF);
    return new_ast_program(p->ar, fn);
}

///////////////////////////////////////////////////
// Parsing statements

ast * parse_function(parser * p) {
    expect_and_consume_token(p, TOK_INT);

    token id_tok = next_token(&p->lex);
    expect_token(id_tok, p, TOK_ID);
    char * name = id_tok.str_val;

    expect_and_consume_token(p, TOK_OPEN_PAR);
    expect_and_consume_token(p, TOK_VOID);
    expect_and_consume_token(p, TOK_CLOSE_PAR);
    expect_and_consume_token(p, TOK_OPEN_BRACE);
    ast * stmt = parse_statement(p);
    expect_ast(stmt, p, AST_STMT);
    expect_and_consume_token(p, TOK_CLOSE_BRACE);
    return new_ast_function(p->ar, name, stmt);
}

ast * parse_statement(parser * p) {
    expect_and_consume_token(p, TOK_RETURN);
    ast * expr = parse_expr(p);
    expect_ast(expr, p, AST_EXPR);
    expect_and_consume_token(p, TOK_SEMICOLON);
    return new_stmt_return(p->ar, expr);
}

///////////////////////////////////////////////////
// Parsing expressions

ast * parse_factor(parser * p) {
    token tok = next_token(&p->lex);
    switch (tok.type) {
        case TOK_INT_LIT:
            return new_expr_constant(p->ar, tok.int_val);
        case TOK_OPEN_PAR: {
            ast * a = parse_expr(p);
            expect_ast(a, p, AST_EXPR);
            expect_and_consume_token(p, TOK_CLOSE_PAR);
            return a;
        }
        case TOK_MINUS:
        case TOK_TILDE: {
            unop_type op = (tok.type == TOK_MINUS) ? UNOP_NEG : UNOP_BIT_NEG;
            ast *     a  = parse_factor(p);
            expect_ast(a, p, AST_EXPR);
            return new_expr_unop(p->ar, op, a);
        }
        default:
            return new_ast_error_expr(p->ar);
    }
    return NULL;
}

ast * parse_expr50(parser * p) {
    ast * lhs = parse_factor(p);
    expect_ast(lhs, p, AST_EXPR);

    token tok = lex_peek(&p->lex);
    while (tok.type == TOK_STAR || tok.type == TOK_SLASH ||
           tok.type == TOK_PERCENT) {
        consume_token(p);
        ast * rhs = parse_factor(p);
        expect_ast(rhs, p, AST_EXPR);
        binop_type op = binop_type_from_token_type(tok.type);
        lhs           = new_expr_binop(p->ar, op, lhs, rhs);
        tok           = lex_peek(&p->lex);
    }
    return lhs;
}

ast * parse_expr45(parser * p) {
    ast * lhs = parse_expr50(p);
    expect_ast(lhs, p, AST_EXPR);

    token tok = lex_peek(&p->lex);
    while (tok.type == TOK_PLUS || tok.type == TOK_MINUS) {
        consume_token(p);
        ast * rhs = parse_expr50(p);
        expect_ast(rhs, p, AST_EXPR);
        binop_type op = binop_type_from_token_type(tok.type);
        lhs           = new_expr_binop(p->ar, op, lhs, rhs);
        tok           = lex_peek(&p->lex);
    }
    return lhs;
}

ast * parse_expr(parser * p) {
    return parse_expr45(p);
}

/////////////////////////////////////////////////////////
// Tests

#ifdef TESTING

void test_parser_1_basic(void) {
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
    assert(a->progr.fn->type == AST_FUNCTION);
    assert(a->progr.fn->fn.body->type == AST_STMT);
    assert(a->progr.fn->fn.name == intern_str(&st, "main"));
    assert(a->progr.fn->fn.body->stmt.type == STMT_RETURN);
    assert(a->progr.fn->fn.body->stmt.ret.expr->type == AST_EXPR);
    assert(a->progr.fn->fn.body->stmt.ret.expr->expr.type == EXPR_UNOP);
    assert(a->progr.fn->fn.body->stmt.ret.expr->expr.unop.op == UNOP_BIT_NEG);
    assert(a->progr.fn->fn.body->stmt.ret.expr->expr.unop.expr->type ==
           AST_EXPR);
    assert(a->progr.fn->fn.body->stmt.ret.expr->expr.unop.expr->expr.type ==
           EXPR_UNOP);
    assert(a->progr.fn->fn.body->stmt.ret.expr->expr.unop.expr->expr.unop.op ==
           UNOP_NEG);
    assert(a->progr.fn->fn.body->stmt.ret.expr->expr.unop.expr->expr.unop.expr
               ->type == AST_EXPR);
    assert(a->progr.fn->fn.body->stmt.ret.expr->expr.unop.expr->expr.unop.expr
               ->expr.type == EXPR_CONST);
    assert(a->progr.fn->fn.body->stmt.ret.expr->expr.unop.expr->expr.unop.expr
               ->expr.value == 105);

    // print_ast(stdout, a, 0);
    free_arena(&ar);
}

void test_parser_3_basic(void) {
    char * text =
        "int main(void)\n"
        "{\n"
        "    return (2*3 + 4/2 - 5%3);\n"
        "}\n";

    arena     ar = make_arena(Mb(8));
    str_store st = make_str_store(&ar, Mb(2), 10007);
    parser    p  = make_parser(text, "test_parser_1_basic", &st, &ar);

    ast * a = parse_program(&p);
    assert(a);
    assert(a->type == AST_PROGRAM);
    assert(a->progr.fn->type == AST_FUNCTION);

    // print_ast(stdout, a, 0);
    free_arena(&ar);
}

void check_it_does_parse(char * text, char * source) {
    arena     ar = make_arena(Mb(8));
    str_store st = make_str_store(&ar, Mb(2), 101);
    parser    p  = make_parser(text, source, &st, &ar);

    ast * program = parse_program(&p);
    assert(program);
    assert(program->type == AST_PROGRAM);
    free_arena(&ar);
}

void test_parser_1_valid(void) {
    char * text =
        " int\n"
        "main\n"
        "(\n"
        "void\n"
        ")\n"
        "{\n"
        "return\n"
        "0\n"
        ";\n"
        "}       \n";
    check_it_does_parse(text, "p_1_valid-1");

    text = "int main(void){return 0;}";
    check_it_does_parse(text, "p_1_valid-2");

    check_it_does_parse(
        "int main(void) {\n"
        "    return 0;\n"
        "}\n",
        "p_1_valid-3");

    check_it_does_parse("int   main    (  void)  {   return  0 ; }",
                        "p_1_valid-4");
    check_it_does_parse(
        "int	main	(	void)	{	return	0	;	}",
        "p_1_valid-5");
}

ast_type check_parse_type(char * text, char * source) {
    arena     ar = make_arena(Mb(4));
    str_store st = make_str_store(&ar, Mb(1), 1001);
    parser    p  = make_parser(text, source, &st, &ar);

    ast * program = parse_program(&p);
    assert(program);
    // print_parse_error(stdout, &p, program);
    ast_type result = program->type;
    free_arena(&ar);
    return result;
}

void test_parser_1_invalid(void) {
    char * text[] = {
        "int main(void) {\n"
        "    return\n",

        "int main(void)\n"
        "{\n"
        "    return 2;\n"
        "}\n"
        "\n"
        "foo\n",

        "int 3 (void) {\n"
        "    return 0;\n"
        "}\n",

        "int main(void) {\n"
        "    RETURN 0;\n"
        "}\n",

        "main(void) {\n"
        "    return 0;\n"
        "}\n",

        "int main(void) {\n"
        "    returns 0;\n"
        "}\n",

        "int main (void) {\n"
        "    return 0\n"
        "}\n",

        "int main(void) {\n"
        "    return int;\n"
        "}\n",

        "int main(void){\n"
        "    retur n 0;\n"
        "}\n",

        "int main )( {\n"
        "    return 0;\n"
        "}\n",

        "int main(void) {\n"
        "    return 0;\n",

        "int main( {\n"
        "    return 0;\n"
        "}\n",
    };

    for_array(i, text) {
        ast_type tp = check_parse_type(text[i], "text1");
        if (tp != AST_ERR) {
            fprintf(stderr, "file %llu parsed, and it shouldn't\n", i);
            assert(tp == AST_PROGRAM);
        }
        assert(tp == AST_ERR);
    }
}

#endif
