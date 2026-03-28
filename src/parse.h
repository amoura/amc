#ifndef PARSE_H
#define PARSE_H

typedef struct {
    lexer lex;
    arena *ar;
} parser;

ast * parse_program(parser *p);
ast * parse_function(parser *p);
ast * parse_expr(parser *p);
ast * parse_statement(parser *p);


#endif // PARSE_H
