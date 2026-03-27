#ifndef LEX_H
#define LEX_H

//gen:enum
typedef enum TokenType {
    TOK_NONE,

    TOK_OPEN_PAR,
    TOK_CLOSE_PAR,
    TOK_OPEN_BRACE,
    TOK_CLOSE_BRACE,
    TOK_SEMICOLON,

    TOK_INT,
    TOK_VOID,
    TOK_RETURN,

    TOK_ID,
    TOK_INT_LIT,

    TOK_EOF,
    TOK_ERR,
} TokenType;

typedef struct {
    TokenType type;
    u64 pos;
    union {
        char *str_val;
        int int_val;
    };
} Token;

typedef struct {
    u32 line;
    u32 col;
} TextPos;

typedef struct {
    char *text;
    char *source;
    u64 len;
    u64 pos;
    StrStore *st;
} Lexer;

Lexer
MakeLexer(char *text, char *source, u64 len, StrStore *st);

Token
NextToken(Lexer *lex);

TextPos
GetTextPos(Lexer *lex);

#endif // LEX_H
