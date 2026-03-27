#include <ctype.h>
#include "lex.h.gen"

struct {
    char *lit;
    char *str;
    TokenType type;
} keywords[] = {
    {"int", NULL, TOK_INT},
    {"void", NULL, TOK_VOID},
    {"return", NULL, TOK_RETURN}
};

TokenType
TokenTypeOfStr(char *str)
{
    for_array(i, keywords) {
        if (keywords[i].str == str) {
            return keywords[i].type;
        }
    }
    return TOK_ID;
}

Token
MakeSimpleToken(TokenType type, u64 pos)
{
    Token tok = {0};
    tok.type = type;
    tok.pos = pos;
    return tok;
}

// We assume ID has been interned
Token
MakeIdToken(char *id, u64 pos)
{
    Token tok = {0};
    tok.type = TOK_ID;
    tok.pos = pos;
    tok.str_val = id;
    return tok;
}

Token
MakeIntToken(int value, u64 pos)
{
    Token tok = {0};
    tok.type = TOK_INT_LIT;
    tok.pos = pos;
    tok.int_val = value;
    return tok;
}

bool
TokenEq(Token a, Token b)
{
    if (a.type != b.type) {
        return false;
    }
    switch (a.type) {
        case TOK_ID:
        case TOK_ERR:
            return a.str_val == b.str_val;
            break;

        case TOK_INT_LIT:
            return a.int_val == b.int_val;
            break;
    }
    return true;
}

const char *
TokenTypeToMsg(TokenType type)
{
    switch (type) {
        case TOK_OPEN_PAR:
            return "'('";
        case TOK_CLOSE_PAR:
            return "')'";
        case TOK_OPEN_BRACE:
            return "'{'";
        case TOK_CLOSE_BRACE:
            return "'}'";
        case TOK_SEMICOLON:
            return "';'";
        case TOK_INT:
            return "'int'";
        case TOK_VOID:
            return "'void'";
        case TOK_RETURN:
            return "'return'";
        case TOK_ID:
            return "identifier";
        case TOK_INT_LIT:
            return "integer literal";
        case TOK_EOF:
            return "end of file";
        case TOK_ERR:
            return "error";
    }
    assert(false);
    return NULL;
}

Lexer
MakeLexer(char *text, char *source, u64 len, StrStore *st)
{
    Lexer lex = {0};
    lex.text = text;
    lex.source = source;
    lex.len = len;
    lex.pos = 0;
    lex.st = st;

    for_array(i, keywords) {
        char *str = InternStr(st, keywords[i].lit);
        keywords[i].str = str;
    }

    return lex;
}

u64
LexerPos(Lexer *lex)
{
    return lex->pos;
}

bool
LexerInBounds(Lexer *lex)
{
    return lex->pos <= lex->len;
}

bool
LexerAtEOF(Lexer *lex)
{
    assert(LexerInBounds(lex));
    return lex->pos == lex->len;
}

char
CurrChar(Lexer *lex)
{
    assert(LexerInBounds(lex));
    return lex->text[lex->pos];
}

void
SkipWhitespace(Lexer *lex)
{
    while (LexerInBounds(lex) && isspace(CurrChar(lex))) {
        lex->pos++;
    }
}

TextPos
CurrTextPos(Lexer *lex)
{
    assert(LexerInBounds(lex));
    u64 pos = 0;
    u32 line = 1;
    u32 col = 1;
    while (pos < lex->pos) {
        if (lex->text[pos] == '\n') {
            line++;
            col = 1;
        } else {
            col++;
        }
        pos++;
    }
    TextPos text_pos = {0};
    text_pos.line = line;
    text_pos.col = col;
    return text_pos;
}

char *
FindNthLine(char *text, int line_num)
{
    for (size_t line=1; *text && line<line_num; text++) {
        if (*text == '\n') {
            line++;
        }
    }
    return text;
}

void
PrintTextRange(FILE *stream, char *beg, char *end)
{
    fwrite(beg, end-beg, 1, stream);
}

void
PrintNChars(FILE *stream, int n, char c)
{
    for_i(int,i,n) {
        fputc(c, stream);
    }
}

void
PrintErrorLocation(FILE *stream, char *text, 
        int start_line, int end_line,  size_t col)
{
    if (start_line < 1) {
        start_line = 1;
    }
    char *beg = FindNthLine(text, start_line);
    char *end = FindNthLine(beg, end_line-start_line+1);
    PrintTextRange(stream, beg, end);
    fputc('\n', stream);
    PrintNChars(stream, col-1, '-');
    fprintf(stream, "^\n");
}

void
LexerError(Lexer *lex, char *msg)
{
    TextPos text_pos = CurrTextPos(lex);
    fprintf(stderr, "%s:%d:%d: lexer error: %s\n",
            lex->source, text_pos.line, text_pos.col, msg);
    exit(1);
}

bool
InnerWordChar(char c)
{
    switch (c) {
        case 'a': case 'b': case 'c': case 'd': case 'e':
        case 'f': case 'g': case 'h': case 'i': case 'j':
        case 'k': case 'l': case 'm': case 'n': case 'o':
        case 'p': case 'q': case 'r': case 's': case 't':
        case 'u': case 'v': case 'w': case 'x': case 'y':
        case 'z':
        case 'A': case 'B': case 'C': case 'D': case 'E':
        case 'F': case 'G': case 'H': case 'I': case 'J':
        case 'K': case 'L': case 'M': case 'N': case 'O':
        case 'P': case 'Q': case 'R': case 'S': case 'T':
        case 'U': case 'V': case 'W': case 'X': case 'Y':
        case 'Z': 
        case '1': case '2': case '3': case '4': case '5':
        case '6': case '7': case '8': case '9': case '0':
        case '_': 
            return true;
            break;
    }
    return false;
}
 

Token
NextToken(Lexer *lex)
{
    Token tok = MakeSimpleToken(TOK_NONE, lex->pos);
    SkipWhitespace(lex);
    assert(LexerInBounds(lex));
    if (LexerAtEOF(lex)) {
        return MakeSimpleToken(TOK_EOF, lex->pos);
    }

    switch (CurrChar(lex)) {
        case '(':
            tok.type = TOK_OPEN_PAR;
            lex->pos++;
            break;

        case ')':
            tok.type = TOK_CLOSE_PAR;
            lex->pos++;
            break;

        case '{':
            tok.type = TOK_OPEN_BRACE;
            lex->pos++;
            break;

        case '}':
            tok.type = TOK_CLOSE_BRACE;
            lex->pos++;
            break;

        case ';':
            tok.type = TOK_SEMICOLON;
            lex->pos++;
            break;

        case '1': case '2': case '3': case '4': case '5':
        case '6': case '7': case '8': case '9': case '0': {
            int value = 0;
            while (LexerInBounds(lex) && isdigit(CurrChar(lex))) {
                int new_value = value*10 + (int)(CurrChar(lex) - '0');
                if (new_value < value) {
                    tok.type = TOK_ERR;
                    tok.str_val = "integer literal overflow";
                    return tok;
                }
                value = new_value;
                lex->pos++;
            }
            if (LexerInBounds(lex) && InnerWordChar(CurrChar(lex))) {
                tok.type = TOK_ERR;
                tok.str_val = "ill-formed numerical literal: expected whitespace";
            } else {
                tok.type = TOK_INT_LIT;
                tok.int_val = value;
            }
        } break;

        case 'a': case 'b': case 'c': case 'd': case 'e':
        case 'f': case 'g': case 'h': case 'i': case 'j':
        case 'k': case 'l': case 'm': case 'n': case 'o':
        case 'p': case 'q': case 'r': case 's': case 't':
        case 'u': case 'v': case 'w': case 'x': case 'y':
        case 'z':
        case 'A': case 'B': case 'C': case 'D': case 'E':
        case 'F': case 'G': case 'H': case 'I': case 'J':
        case 'K': case 'L': case 'M': case 'N': case 'O':
        case 'P': case 'Q': case 'R': case 'S': case 'T':
        case 'U': case 'V': case 'W': case 'X': case 'Y':
        case 'Z': 
        case '_': {
            u64 pos0 = lex->pos;
            while (LexerInBounds(lex) && InnerWordChar(CurrChar(lex))) {
                lex->pos++;
            }
            char *str = InternStrN(lex->st, lex->text+pos0, lex->pos-pos0);
            TokenType type = TokenTypeOfStr(str);
            tok.type = type;
            if (type == TOK_ID) {
                tok.str_val = str;
            }
        } break;

        default:
            tok.type = TOK_ERR;
            tok.str_val = "invalid token";
            break;
    }

    assert(tok.type != TOK_NONE);
    return tok;
}


//////////////////////////////////////////////////////////////////
// Tests

#ifdef TESTING

bool
IsNextToken(Lexer *lex, Token tok)
{
    Token next_tok = NextToken(lex);
    return TokenEq(next_tok, tok);
}

void
RequireNextToken(Lexer *lex, Token tok)
{
    assert(IsNextToken(lex, tok));
}

void test_lexer_1()
{
    char text[] =
        "int main(void) {\n"
        "    return 102;\n"
        "}\n";

    Arena arena = MakeArena(Mb(8));
    StrStore st = MakeStrStore(&arena, Mb(2), 101);
    Lexer lex = MakeLexer(text, "tst.c", strlen(text), &st);

    Token tok = NextToken(&lex);
    assert(tok.type == TOK_INT);
    
    tok = NextToken(&lex);
    assert(tok.type == TOK_ID);
    assert(StrEq(tok.str_val, "main"));

    tok = NextToken(&lex);
    assert(tok.type == TOK_OPEN_PAR);

    tok = NextToken(&lex);
    assert(tok.type == TOK_VOID);

    tok = NextToken(&lex);
    assert(tok.type == TOK_CLOSE_PAR);

    tok = NextToken(&lex);
    assert(tok.type == TOK_OPEN_BRACE);

    tok = NextToken(&lex);
    assert(tok.type == TOK_RETURN);

    tok = NextToken(&lex);
    assert(tok.type == TOK_INT_LIT);
    assert(tok.int_val == 102);

    tok = NextToken(&lex);
    assert(tok.type == TOK_SEMICOLON);

    tok = NextToken(&lex);
    assert(tok.type == TOK_CLOSE_BRACE);

    tok = NextToken(&lex);
    assert(tok.type == TOK_EOF);

    FreeArena(&arena);
}

void
CheckLexing(char *text, TokenType *types, u64 num_tokens)
{
    Arena arena = MakeArena(Mb(8));
    StrStore st = MakeStrStore(&arena, Mb(2), 101);
    Lexer lex = MakeLexer(text, "tst.c", strlen(text), &st);

    u64 i = 0;
    Token tok = {0};
    do {
        tok = NextToken(&lex);
        if (tok.type != types[i]) {
            fprintf(stderr, "token %llu: found %s, should be %s\n",
                    i, TokenTypeToStr(tok.type), TokenTypeToStr(types[i]));
            exit(1);
        }
        i++;
    } while (tok.type!=TOK_EOF && tok.type!=TOK_ERR);
    if (i != num_tokens) {
        assert(i < num_tokens);
        fprintf(stderr, "expected %llu tokens, found %llu\n",
                num_tokens, i);
        exit(1);
    }

    FreeArena(&arena);
}

void
CheckItDoesLex(char *text)
{
    Arena arena = MakeArena(Mb(8));
    StrStore st = MakeStrStore(&arena, Mb(2), 101);
    Lexer lex = MakeLexer(text, "tst.c", strlen(text), &st);

    Token tok = {0};
    do {
        tok = NextToken(&lex);
    } while (tok.type!=TOK_EOF && tok.type!=TOK_ERR);
    if (tok.type != TOK_EOF) {
        assert(tok.type == TOK_ERR);
        fprintf(stderr, "failed to lex\n");
        exit(1);
    }
}

void test_lexer_1_invalid(void)
{
    char *text =
        "int main(void) {\n"
        "return 0@1;\n"
        "}\n";
    TokenType types[] = {
        TOK_INT, TOK_ID, TOK_OPEN_PAR, TOK_VOID, TOK_CLOSE_PAR, TOK_OPEN_BRACE,
        TOK_RETURN, TOK_INT_LIT, TOK_ERR
    };
    CheckLexing(text, types, ArrayLen(types));

    text = "\n\\";
    TokenType tps[] = {TOK_ERR};
    CheckLexing(text, tps, ArrayLen(tps));

    text = "\n`";
    CheckLexing(text, tps, ArrayLen(tps));

    text =
        "int main(void) {\n"
        "return 1foo;\n"
        "}\n";
    TokenType tps1[] = {
        TOK_INT, TOK_ID, TOK_OPEN_PAR, TOK_VOID, TOK_CLOSE_PAR, TOK_OPEN_BRACE,
        TOK_RETURN, TOK_ERR
    };
    CheckLexing(text, tps1, ArrayLen(tps1));

    text =
        "int main(void)\n"
        "{\n"
        "    return @b;\n"
        "}";
    TokenType tps2[] = {
        TOK_INT, TOK_ID, TOK_OPEN_PAR, TOK_VOID, TOK_CLOSE_PAR, TOK_OPEN_BRACE,
        TOK_RETURN, TOK_ERR
    };
    CheckLexing(text, tps2, ArrayLen(tps2));
}

void test_lexer_1_valid(void)
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
    CheckItDoesLex(text);

    text = "int main(void){return 0;}";
    CheckItDoesLex(text);

    CheckItDoesLex(
        "int main(void) {\n"
        "    return 0;\n"
        "}\n");

    CheckItDoesLex("int   main    (  void)  {   return  0 ; }");
    CheckItDoesLex("int	main	(	void)	{	return	0	;	}");
}


#endif // TESTING
