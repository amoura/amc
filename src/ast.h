#ifndef AST_H
#define AST_H

//gen:enum
typedef enum AstType {
    AST_NONE,
    AST_PROGRAM,
    AST_FUNCTION,
    AST_STMT,
    AST_EXPR,
    AST_ERR,
} AstType;

//gen:enum
typedef enum ExprType {
    EXPR_NONE,
    EXPR_CONST,
} ExprType;

//gen:enum
typedef enum StmtType {
    STMT_NONE,
    STMT_RETURN,
} StmtType;

//gen:enum
typedef enum AstErrorType {
    AST_ERR_NONE,
    AST_ERR_EXP_TOKEN,
    AST_ERR_EXP_AST,
} AstErrorType;


struct Ast;
typedef struct Ast Ast;

typedef struct {
    Ast *fn;
} AstProgram;

typedef struct {
    char *name;
    Ast *body;
} AstFunction;

typedef struct {
    ExprType type;
    int value;
} AstExpr;

typedef struct {
    Ast *expr;
} StmtReturn;

typedef struct {
    StmtType type;
    StmtReturn ret;
} AstStmt;

typedef struct AstError {
    struct Ast *next;
    AstErrorType type;
    union {
        TokenType exp_token_type;
        AstType exp_ast_type;
    };
    TokenType found_tok_type;
} AstError;

struct Ast {
    AstType type;
    union {
        AstProgram progr;
        AstFunction fn;
        AstExpr expr;
        AstStmt stmt;
        AstError err;
    };
};


#endif // AST_H
