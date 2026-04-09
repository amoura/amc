#ifndef LEX_H
#define LEX_H

// gen:enum
typedef enum token_type {
    TOK_NONE,

    TOK_MINUS,
    TOK_MINUS_MINUS,
    TOK_TILDE,

    TOK_OPEN_PAR,
    TOK_CLOSE_PAR,
    TOK_OPEN_BRACE,
    TOK_CLOSE_BRACE,
    TOK_SEMICOLON,

    TOK_STAR,
    TOK_SLASH,
    TOK_PLUS,
    TOK_PERCENT,

    TOK_INT,
    TOK_VOID,
    TOK_RETURN,

    TOK_ID,
    TOK_INT_LIT,

    TOK_EOF,
    TOK_ERR,
} token_type;

typedef struct {
    token_type type;
    u64        pos;
    union {
        char * str_val;
        int    int_val;
    };
} token;

typedef struct {
    u32 line;
    u32 col;
} text_pos;

typedef struct {
    char *      text;
    char *      source;
    u64         len;
    u64         pos;
    str_store * st;
} lexer;

lexer    make_lexer(char * text, char * source, u64 len, str_store * st);
token    next_token(lexer * lex);
token    lex_peek(lexer * lex);
text_pos get_text_pos(lexer * lex);

#endif  // LEX_H
