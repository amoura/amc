#include "ast.h.gen"

bool
ast_is_err(ast *a)
{
    return a->type == AST_ERR;
}

const char *
ast_type_to_msg(ast_type type)
{
    switch (type) {
        case AST_PROGRAM:
            return "program";
        case AST_FUNCTION:
            return "function";
        case AST_STMT:
            return "statement";
        case AST_EXPR:
            return "expression";
        case AST_ERR:
            return "error";
    }
    assert(false);
    return NULL;
}
    

////////////////////////////////////////////////
// Constructors

ast *
new_ast_program(arena *ar, ast *fn)
{
    assert(ar);
    assert(fn);
    assert(fn->type == AST_FUNCTION);
    ast *a = arena_alloc_type(ar, ast);
    a->type = AST_PROGRAM;
    a->progr.fn = fn;
    return a;
}

ast *
new_ast_function(arena *ar, char *name, ast *body)
{
    assert(ar);
    assert(name);
    assert(body);
    ast *a = arena_alloc_type(ar, ast);
    a->type = AST_FUNCTION;
    a->fn.name = name;
    a->fn.body = body;
    return a;
}

ast *
new_ast_error_exp_token(
        arena *ar,
        token_type exp_tok_type, 
        token_type found_tok_type
) {
    assert(ar);
    ast *a = arena_alloc_type(ar, ast);
a->type = AST_ERR;
    a->err.type = AST_ERR_EXP_TOKEN;
    a->err.next = NULL;
    a->err.exp_token_type = exp_tok_type;
    a->err.found_tok_type = found_tok_type;
    return a;
}

ast *
new_ast_error_exp_ast(
        arena *ar,
        ast *next,
        ast_type exp_ast_type
) {
    assert(ar);
    ast *a = arena_alloc_type(ar, ast);
    a->type = AST_ERR;
    a->err.next = next;
    a->err.type = AST_ERR_EXP_AST;
    a->err.exp_ast_type = exp_ast_type;
    return a;
}

ast *
new_stmt_return(arena *ar, ast *expr)
{
    assert(ar);
    assert(expr);
    assert(expr->type == AST_EXPR);
    ast *a = arena_alloc_type(ar, ast);
    a->type = AST_STMT;
    a->stmt.type = STMT_RETURN;
    a->stmt.ret.expr = expr;
    return a;
}

ast *
new_expr_constant(arena *ar, int value)
{
    assert(ar);
    ast *a = arena_alloc_type(ar, ast);
    a->type = AST_EXPR;
    a->expr.type = EXPR_CONST;
    a->expr.value = value;
    return a;
}


///////////////////////////////////////////////////
// Pretty-printing

void
print_indent(FILE *stream, int indent)
{
    for_i(int,i,indent) {
        fprintf(stream, " ");
    }
}

void
print_ast(FILE *stream, ast *a, int indent)
{
    assert(stream);
    assert(a);

    print_indent(stream, indent);
    switch (a->type) {
        case AST_PROGRAM:
            fprintf(stream, "Program:\n");
            print_ast(stream, a->progr.fn, indent+4);
            break;

        case AST_FUNCTION:
            fprintf(stream, "Function:\n");
            print_indent(stream, indent+4);
            fprintf(stream, "name: %s\n", a->fn.name);
            print_ast(stream, a->fn.body, indent+4);
            break;

        case AST_STMT:
            fprintf(stream, "Statement (%s):\n", stmt_type_to_str(a->stmt.type));
            print_ast(stream, a->stmt.ret.expr, indent+4);
            break;

        case AST_EXPR:
            fprintf(stream, "Expr (%s):\n", expr_type_to_str(a->expr.type));
            print_indent(stream, indent+4);
            fprintf(stream, "value: %d\n", a->expr.value);
            break;

        default:
            assert(false);
    }
}


//////////////////////////////////////////////////////////
// Tests

#ifdef TESTING

void
test_ast_1(void)
{
    arena ar = make_arena(Mb(1));
    ast *int_const = new_expr_constant(&ar, 102);
    assert(int_const);
    ast *ret = new_stmt_return(&ar, int_const);
    assert(ret);
    ast *fn = new_ast_function(&ar, "main", ret);
    assert(fn);
    ast *progr = new_ast_program(&ar, fn);
    assert(progr);

    assert(int_const->type == AST_EXPR);
    assert(int_const->expr.type == EXPR_CONST);
    assert(progr->type == AST_PROGRAM);
    assert(progr->progr.fn == fn);
    assert(fn->type == AST_FUNCTION);
    assert(fn->fn.body == ret);
    assert(ret->type == AST_STMT);
    assert(ret->stmt.type == STMT_RETURN);
    assert(ret->stmt.ret.expr == int_const);
    
    //print_ast(stdout, progr, 0);
    free_arena(&ar);
}

#endif
