Parser
MakeParser(char *text, char *source, StrStore *st, Arena *arena)
{
    Lexer lexer = MakeLexer(text, source, strlen(text), st);
    Parser parser = {0};
    parser.lexer = lexer;
    parser.arena = arena;
    return parser;
}

void
PrintParseError(FILE *stream, Parser *p, Ast *ast)
{
    assert(stream);
    assert(p);

    TextPos pos = CurrTextPos(&p->lexer);
    PrintErrorLocation(stream, p->lexer.text, pos.line-3, pos.line, pos.col);
    for (int depth=0; ast; ast=ast->err.next, depth++) {
        assert(ast->type == AST_ERR);

        char *msg;
        if (depth == 0) {
            msg = "parsing error";
        } else {
            msg = ".............";
        }

        fprintf(stream, "%s:%d:%d: %s (%d): ",
                p->lexer.source, pos.line, pos.col, msg, depth);
        switch (ast->err.type) {
            case AST_ERR_EXP_TOKEN:
                fprintf(stream, "expected token %s, found %s\n",
                        TokenTypeToMsg(ast->err.exp_token_type),
                        TokenTypeToMsg(ast->err.found_tok_type));
                break;

            case AST_ERR_EXP_AST:
                fprintf(stream, "expected %s\n",
                        AstTypeToMsg(ast->err.exp_ast_type));
                break;

            default:
                assert(false);
        }
    }
}

#define ExpectAst(ast,p,ast_type)  \
    do {\
        if (AstIsErr(ast)) {\
            return NewAstErrorExpAst(p->arena, ast, ast_type);\
        }\
    } while (false)

#define ExpectToken(tok,p,exp_token_type)  \
    do {\
        if (tok.type != exp_token_type) {\
            return NewAstErrorExpToken(p->arena, exp_token_type, tok.type);\
        }\
    } while (false)

#define ExpectAndConsumeToken(p,exp_token_type)  \
    do {\
        Token tok_ = NextToken(&p->lexer);\
        if (tok_.type != exp_token_type) {\
            return NewAstErrorExpToken(p->arena, exp_token_type, tok_.type);\
        }\
    } while (false)


/////////////////////////////////////////////////////////
// Recursive descent

Ast *
ParseProgram(Parser *p)
{
    Ast *fn = ParseFunction(p);
    ExpectAst(fn, p, AST_FUNCTION);
    ExpectAndConsumeToken(p, TOK_EOF);
    return NewAstProgram(p->arena, fn);
}

Ast *
ParseFunction(Parser *p)
{
    ExpectAndConsumeToken(p, TOK_INT);

    Token id_tok = NextToken(&p->lexer);
    ExpectToken(id_tok, p, TOK_ID);
    char *name = id_tok.str_val;

    ExpectAndConsumeToken(p, TOK_OPEN_PAR);
    ExpectAndConsumeToken(p, TOK_VOID);
    ExpectAndConsumeToken(p, TOK_CLOSE_PAR);
    ExpectAndConsumeToken(p, TOK_OPEN_BRACE);
    Ast *stmt = ParseStatement(p);
    ExpectAst(stmt, p, AST_STMT);
    ExpectAndConsumeToken(p, TOK_CLOSE_BRACE);
    return NewAstFunction(p->arena, name, stmt);
}

Ast *
ParseStatement(Parser *p)
{
    ExpectAndConsumeToken(p, TOK_RETURN);
    Ast *expr = ParseExpr(p);
    ExpectAst(expr, p, AST_EXPR);
    ExpectAndConsumeToken(p, TOK_SEMICOLON);
    return NewStmtReturn(p->arena, expr);
}

Ast *
ParseExpr(Parser *p)
{
    Token tok = NextToken(&p->lexer);
    ExpectToken(tok, p, TOK_INT_LIT);
    assert(tok.type == TOK_INT_LIT);
    return NewExprConstant(p->arena, tok.int_val);
}


/////////////////////////////////////////////////////////
// Tests

#ifdef TESTING

void 
test_parser_1_basic(void)
{
    char *text =
        "int main(void)\n"
        "{\n"
        "    return 105;\n"
        "}\n";

    Arena arena = MakeArena(Mb(8));
    StrStore st = MakeStrStore(&arena, Mb(2), 10007);
    Parser parser = MakeParser(text, "test_parser_1_basic", &st, &arena);

    Ast *ast = ParseProgram(&parser);
    assert(ast);
    assert(ast->type == AST_PROGRAM);
    assert(ast->progr.fn->type == AST_FUNCTION);
    assert(ast->progr.fn->fn.body->type == AST_STMT);
    assert(ast->progr.fn->fn.name == InternStr(&st, "main"));
    assert(ast->progr.fn->fn.body->stmt.type == STMT_RETURN);
    assert(ast->progr.fn->fn.body->stmt.ret.expr->type == AST_EXPR);
    assert(ast->progr.fn->fn.body->stmt.ret.expr->expr.type == EXPR_CONST);
    assert(ast->progr.fn->fn.body->stmt.ret.expr->expr.value == 105);

    //PrintAst(stdout, ast, 0);

    FreeArena(&arena);
}

void
CheckItDoesParse(char *text, char *source)
{
    Arena arena = MakeArena(Mb(8));
    StrStore st = MakeStrStore(&arena, Mb(2), 101);
    Parser p = MakeParser(text, source, &st, &arena);

    Ast *program = ParseProgram(&p);
    assert(program);
    assert(program->type == AST_PROGRAM);
    FreeArena(&arena);
}

void 
test_parser_1_valid(void)
{
    char *text =
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
    CheckItDoesParse(text, "parser_1_valid-1");

    text = "int main(void){return 0;}";
    CheckItDoesParse(text, "parser_1_valid-2");

    CheckItDoesParse(
        "int main(void) {\n"
        "    return 0;\n"
        "}\n", "parser_1_valid-3");

    CheckItDoesParse("int   main    (  void)  {   return  0 ; }", "parser_1_valid-4");
    CheckItDoesParse("int	main	(	void)	{	return	0	;	}", "parser_1_valid-5");
}

AstType
CheckParseType(char *text, char *source)
{    
    Arena arena = MakeArena(Mb(4));
    StrStore st = MakeStrStore(&arena, Mb(1), 1001);
    Parser p = MakeParser(text, source, &st, &arena);

    Ast *program = ParseProgram(&p);
    assert(program);
    //PrintParseError(stdout, &p, program);
    AstType result = program->type;
    FreeArena(&arena);
    return result;
}


void test_parser_1_invalid(void)
{    
    char *text[] = {
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

    for_array(i,text) {
        AstType tp = CheckParseType(text[i], "text1");
        if (tp != AST_ERR) {
            fprintf(stderr, "file %llu parsed, and it shouldn't\n", i);
            assert(tp == AST_PROGRAM);
        }
        assert(tp == AST_ERR);
    }
}

#endif

