#ifndef AST_H
#define AST_H

// gen:enum
typedef enum ast_type {
    AST_NONE,
    AST_PROGRAM,
    AST_FUNCTION,
    AST_STMT,
    AST_EXPR,
    AST_ERR,
} ast_type;

// gen:enum
typedef enum expr_type {
    EXPR_NONE,
    EXPR_CONST,
    EXPR_UNOP,
    EXPR_BINOP,
} expr_type;

// gen:enum
typedef enum stmt_type {
    STMT_NONE,
    STMT_RETURN,
} stmt_type;

// gen:enum
typedef enum unop_type {
    UNOP_NONE,
    UNOP_NEG,
    UNOP_BIT_NEG,
} unop_type;

// gen:enum
typedef enum binop_type {
    BINOP_NONE,
    BINOP_MINUS,
    BINOP_PLUS,
    BINOP_MUL,
    BINOP_DIV,
    BINOP_REM,
} binop_type;

// gen:enum
typedef enum ast_error_type {
    AST_ERR_NONE,
    AST_ERR_EXP_TOKEN,
    AST_ERR_EXP_AST,
    AST_ERR_EXPR,
} ast_error_type;

typedef struct ast ast;

typedef struct {
    ast * fn;
} ast_program;

typedef struct {
    char * name;
    ast *  body;
} ast_function;

/////////////////////////////////////////////////////
// Expressions

typedef struct ast_expr ast_expr;

typedef struct {
    unop_type op;
    ast *     expr;
} expr_unop;

typedef struct {
    binop_type op;
    ast *      lhs;
    ast *      rhs;
} expr_binop;

struct ast_expr {
    expr_type type;
    union {
        int        value;
        expr_unop  unop;
        expr_binop binop;
    };
};

///////////////////////////////////////////////////
// Statements

typedef struct {
    ast * expr;
} stmt_return;

typedef struct {
    stmt_type   type;
    stmt_return ret;
} ast_stmt;

////////////////////////////////////////////////////

typedef struct ast_error {
    struct ast *   next;
    ast_error_type type;
    union {
        token_type exp_token_type;
        ast_type   exp_ast_type;
    };
    token_type found_tok_type;
} ast_error;

struct ast {
    ast_type type;
    union {
        ast_program  progr;
        ast_function fn;
        ast_expr     expr;
        ast_stmt     stmt;
        ast_error    err;
    };
};

#endif  // AST_H
