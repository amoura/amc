#include "ast.h.gen"

bool
AstIsErr(Ast *ast)
{
    return ast->type == AST_ERR;
}

const char *
AstTypeToMsg(AstType type)
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

Ast *
NewAstProgram(Arena *arena, Ast *fn)
{
    assert(arena);
    assert(fn);
    assert(fn->type == AST_FUNCTION);
    Ast *ast = ArenaAllocType(arena, Ast);
    ast->type = AST_PROGRAM;
    ast->progr.fn = fn;
    return ast;
}

Ast *
NewAstFunction(Arena *arena, char *name, Ast *body)
{
    assert(arena);
    assert(name);
    assert(body);
    Ast *ast = ArenaAllocType(arena, Ast);
    ast->type = AST_FUNCTION;
    ast->fn.name = name;
    ast->fn.body = body;
    return ast;
}

Ast *
NewAstErrorExpToken(
        Arena *arena,
        TokenType exp_tok_type, 
        TokenType found_tok_type
) {
    assert(arena);
    Ast *ast = ArenaAllocType(arena, Ast);
ast->type = AST_ERR;
    ast->err.type = AST_ERR_EXP_TOKEN;
    ast->err.next = NULL;
    ast->err.exp_token_type = exp_tok_type;
    ast->err.found_tok_type = found_tok_type;
    return ast;
}

Ast *
NewAstErrorExpAst(
        Arena *arena,
        Ast *next,
        AstType exp_ast_type
) {
    assert(arena);
    Ast *ast = ArenaAllocType(arena, Ast);
    ast->type = AST_ERR;
    ast->err.next = next;
    ast->err.type = AST_ERR_EXP_AST;
    ast->err.exp_ast_type = exp_ast_type;
    return ast;
}

Ast *
NewStmtReturn(Arena *arena, Ast *expr)
{
    assert(arena);
    assert(expr);
    assert(expr->type == AST_EXPR);
    Ast *ast = ArenaAllocType(arena, Ast);
    ast->type = AST_STMT;
    ast->stmt.type = STMT_RETURN;
    ast->stmt.ret.expr = expr;
    return ast;
}

Ast *
NewExprConstant(Arena *arena, int value)
{
    assert(arena);
    Ast *ast = ArenaAllocType(arena, Ast);
    ast->type = AST_EXPR;
    ast->expr.type = EXPR_CONST;
    ast->expr.value = value;
    return ast;
}


///////////////////////////////////////////////////
// Pretty-printing

void
PrintIndent(FILE *stream, int indent)
{
    for_i(int,i,indent) {
        fprintf(stream, " ");
    }
}

void
PrintAst(FILE *stream, Ast *ast, int indent)
{
    assert(stream);
    assert(ast);

    PrintIndent(stream, indent);
    switch (ast->type) {
        case AST_PROGRAM:
            fprintf(stream, "Program:\n");
            PrintAst(stream, ast->progr.fn, indent+4);
            break;

        case AST_FUNCTION:
            fprintf(stream, "Function:\n");
            PrintIndent(stream, indent+4);
            fprintf(stream, "name: %s\n", ast->fn.name);
            PrintAst(stream, ast->fn.body, indent+4);
            break;

        case AST_STMT:
            fprintf(stream, "Statement (%s):\n", StmtTypeToStr(ast->stmt.type));
            PrintAst(stream, ast->stmt.ret.expr, indent+4);
            break;

        case AST_EXPR:
            fprintf(stream, "Expr (%s):\n", ExprTypeToStr(ast->expr.type));
            PrintIndent(stream, indent+4);
            fprintf(stream, "value: %d\n", ast->expr.value);
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
    Arena arena = MakeArena(Mb(1));
    Ast *int_const = NewExprConstant(&arena, 102);
    assert(int_const);
    Ast *ret = NewStmtReturn(&arena, int_const);
    assert(ret);
    Ast *fn = NewAstFunction(&arena, "main", ret);
    assert(fn);
    Ast *progr = NewAstProgram(&arena, fn);
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
    
    //PrintAst(stdout, progr, 0);
    FreeArena(&arena);
}

#endif
