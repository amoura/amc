#ifndef AST_H
#define AST_H

//gen:enum
typedef enum ast_type {
    AST_NONE,
    AST_PROGRAM,
    AST_FUNCTION,
    AST_STMT,
    AST_EXPR,
    AST_ERR,
} ast_type;

//gen:enum
typedef enum expr_type {
    EXPR_NONE,
    EXPR_CONST,
} expr_type;

//gen:enum
typedef enum stmt_type {
    STMT_NONE,
    STMT_RETURN,
} stmt_type;

//gen:enum
typedef enum ast_error_type {
    AST_ERR_NONE,
    AST_ERR_EXP_TOKEN,
    AST_ERR_EXP_AST,
} ast_error_type;

typedef struct ast ast;

typedef struct {
    ast *fn;
} ast_program;

typedef struct {
    char *name;
    ast *body;
} ast_function;

typedef struct {
    expr_type type;
    int value;
} ast_expr;

typedef struct {
    ast *expr;
} stmt_return;

typedef struct {
    stmt_type type;
    stmt_return ret;
} ast_stmt;

typedef struct ast_error {
    struct ast *next;
    ast_error_type type;
    union {
        token_type exp_token_type;
        ast_type exp_ast_type;
    };
    token_type found_tok_type;
} ast_error;

struct ast {
    ast_type type;
    union {
        ast_program progr;
        ast_function fn;
        ast_expr expr;
        ast_stmt stmt;
        ast_error err;
    };
};


#endif // AST_H
