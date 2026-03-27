#ifndef PARSE_H
#define PARSE_H

typedef struct {
    Lexer lexer;
    Arena *arena;
} Parser;

Ast * ParseProgram(Parser *p);
Ast * ParseFunction(Parser *p);
Ast * ParseExpr(Parser *p);
Ast * ParseStatement(Parser *p);


#endif // PARSE_H
