#include <ctype.h>
#include "lex.h.gen"

struct {
    char *     lit;
    char *     str;
    token_type type;
} keywords[] = {
    {"int",    NULL, TOK_INT   },
    {"void",   NULL, TOK_VOID  },
    {"return", NULL, TOK_RETURN}
};

token_type token_type_of_str(char * str) {
    for_array(i, keywords) {
        if (keywords[i].str == str) {
            return keywords[i].type;
        }
    }
    return TOK_ID;
}

token make_simple_token(token_type type, u64 pos) {
    token tok = {0};
    tok.type  = type;
    tok.pos   = pos;
    return tok;
}

// We assume ID has been interned
token make_id_token(char * id, u64 pos) {
    token tok   = {0};
    tok.type    = TOK_ID;
    tok.pos     = pos;
    tok.str_val = id;
    return tok;
}

token make_int_token(int value, u64 pos) {
    token tok   = {0};
    tok.type    = TOK_INT_LIT;
    tok.pos     = pos;
    tok.int_val = value;
    return tok;
}

bool token_eq(token a, token b) {
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

        default:
            return true;
    }
    return true;
}

const char * token_type_to_msg(token_type type) {
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
        case TOK_MINUS:
            return "'-'";
        case TOK_MINUS_MINUS:
            return "'--'";
        case TOK_TILDE:
            return "'~'";
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
        default:
            assert(false);
    }
    return NULL;
}

lexer make_lexer(char * text, char * source, u64 len, str_store * st) {
    lexer lex  = {0};
    lex.text   = text;
    lex.source = source;
    lex.len    = len;
    lex.pos    = 0;
    lex.st     = st;

    for_array(i, keywords) {
        char * str      = intern_str(st, keywords[i].lit);
        keywords[i].str = str;
    }

    return lex;
}

u64 lexer_pos(lexer * lex) {
    return lex->pos;
}

bool lexer_in_bounds(lexer * lex) {
    return lex->pos <= lex->len;
}

bool lexer_at_eof(lexer * lex) {
    assert(lexer_in_bounds(lex));
    return lex->pos == lex->len;
}

char curr_char(lexer * lex) {
    assert(lexer_in_bounds(lex));
    return lex->text[lex->pos];
}

void skip_whitespace(lexer * lex) {
    while (lexer_in_bounds(lex) && isspace(curr_char(lex))) {
        lex->pos++;
    }
}

text_pos curr_text_pos(lexer * lex) {
    assert(lexer_in_bounds(lex));
    u64 pos  = 0;
    u32 line = 1;
    u32 col  = 1;
    while (pos < lex->pos) {
        if (lex->text[pos] == '\n') {
            line++;
            col = 1;
        } else {
            col++;
        }
        pos++;
    }
    text_pos text_pos = {0};
    text_pos.line     = line;
    text_pos.col      = col;
    return text_pos;
}

char * find_nth_line(char * text, int line_num) {
    for (size_t line = 1; *text && line < line_num; text++) {
        if (*text == '\n') {
            line++;
        }
    }
    return text;
}

void print_text_range(FILE * stream, char * beg, char * end) {
    fwrite(beg, end - beg, 1, stream);
}

void print_n_chars(FILE * stream, int n, char c) {
    for_i(int, i, n) {
        fputc(c, stream);
    }
}

void print_error_location(FILE * stream,
                          char * text,
                          int    start_line,
                          int    end_line,
                          size_t col) {
    if (start_line < 1) {
        start_line = 1;
    }
    char * beg = find_nth_line(text, start_line);
    char * end = find_nth_line(beg, end_line - start_line + 1);
    print_text_range(stream, beg, end);
    fputc('\n', stream);
    print_n_chars(stream, col - 1, '-');
    fprintf(stream, "^\n");
}

void lexer_error(lexer * lex, char * msg) {
    text_pos text_pos = curr_text_pos(lex);
    fprintf(stderr,
            "%s:%d:%d: lexer error: %s\n",
            lex->source,
            text_pos.line,
            text_pos.col,
            msg);
    exit(1);
}

bool inner_word_char(char c) {
    switch (c) {
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
        case 'g':
        case 'h':
        case 'i':
        case 'j':
        case 'k':
        case 'l':
        case 'm':
        case 'n':
        case 'o':
        case 'p':
        case 'q':
        case 'r':
        case 's':
        case 't':
        case 'u':
        case 'v':
        case 'w':
        case 'x':
        case 'y':
        case 'z':
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
        case 'G':
        case 'H':
        case 'I':
        case 'J':
        case 'K':
        case 'L':
        case 'M':
        case 'N':
        case 'O':
        case 'P':
        case 'Q':
        case 'R':
        case 'S':
        case 'T':
        case 'U':
        case 'V':
        case 'W':
        case 'X':
        case 'Y':
        case 'Z':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '0':
        case '_':
            return true;
            break;
    }
    return false;
}

token next_token(lexer * lex) {
    token tok = make_simple_token(TOK_NONE, lex->pos);
    skip_whitespace(lex);
    assert(lexer_in_bounds(lex));
    if (lexer_at_eof(lex)) {
        return make_simple_token(TOK_EOF, lex->pos);
    }

    switch (curr_char(lex)) {
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

        case '-':
            lex->pos++;
            if (!lexer_at_eof(lex) && curr_char(lex) == '-') {
                lex->pos++;
                tok.type = TOK_MINUS_MINUS;
            } else {
                tok.type = TOK_MINUS;
            }
            break;

        case '~':
            tok.type = TOK_TILDE;
            lex->pos++;
            break;

        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '0': {
            int value = 0;
            while (lexer_in_bounds(lex) && isdigit(curr_char(lex))) {
                int new_value = value * 10 + (int)(curr_char(lex) - '0');
                if (new_value < value) {
                    tok.type    = TOK_ERR;
                    tok.str_val = "integer literal overflow";
                    return tok;
                }
                value = new_value;
                lex->pos++;
            }
            if (lexer_in_bounds(lex) && inner_word_char(curr_char(lex))) {
                tok.type = TOK_ERR;
                tok.str_val =
                    "ill-formed numerical literal: expected whitespace";
            } else {
                tok.type    = TOK_INT_LIT;
                tok.int_val = value;
            }
        } break;

        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
        case 'g':
        case 'h':
        case 'i':
        case 'j':
        case 'k':
        case 'l':
        case 'm':
        case 'n':
        case 'o':
        case 'p':
        case 'q':
        case 'r':
        case 's':
        case 't':
        case 'u':
        case 'v':
        case 'w':
        case 'x':
        case 'y':
        case 'z':
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
        case 'G':
        case 'H':
        case 'I':
        case 'J':
        case 'K':
        case 'L':
        case 'M':
        case 'N':
        case 'O':
        case 'P':
        case 'Q':
        case 'R':
        case 'S':
        case 'T':
        case 'U':
        case 'V':
        case 'W':
        case 'X':
        case 'Y':
        case 'Z':
        case '_': {
            u64 pos0 = lex->pos;
            while (lexer_in_bounds(lex) && inner_word_char(curr_char(lex))) {
                lex->pos++;
            }
            char * str =
                intern_str_n(lex->st, lex->text + pos0, lex->pos - pos0);
            token_type type = token_type_of_str(str);
            tok.type        = type;
            if (type == TOK_ID) {
                tok.str_val = str;
            }
        } break;

        default:
            tok.type    = TOK_ERR;
            tok.str_val = "invalid token";
            break;
    }

    assert(tok.type != TOK_NONE);
    return tok;
}

//////////////////////////////////////////////////////////////////
// Tests

#ifdef TESTING

bool is_next_token(lexer * lex, token tok) {
    token next_tok = next_token(lex);
    return token_eq(next_tok, tok);
}

void require_next_token(lexer * lex, token tok) {
    assert(is_next_token(lex, tok));
}

void test_lexer_1() {
    char text[] =
        "int main(void) {\n"
        "    return ~(-102);\n"
        "}\n";

    arena     ar  = make_arena(Mb(8));
    str_store st  = make_str_store(&ar, Mb(2), 101);
    lexer     lex = make_lexer(text, "tst.c", strlen(text), &st);

    token tok = next_token(&lex);
    assert(tok.type == TOK_INT);

    tok = next_token(&lex);
    assert(tok.type == TOK_ID);
    assert(str_eq(tok.str_val, "main"));

    tok = next_token(&lex);
    assert(tok.type == TOK_OPEN_PAR);

    tok = next_token(&lex);
    assert(tok.type == TOK_VOID);

    tok = next_token(&lex);
    assert(tok.type == TOK_CLOSE_PAR);

    tok = next_token(&lex);
    assert(tok.type == TOK_OPEN_BRACE);

    tok = next_token(&lex);
    assert(tok.type == TOK_RETURN);

    tok = next_token(&lex);
    assert(tok.type == TOK_TILDE);

    tok = next_token(&lex);
    assert(tok.type == TOK_OPEN_PAR);

    tok = next_token(&lex);
    assert(tok.type == TOK_MINUS);

    tok = next_token(&lex);
    assert(tok.type == TOK_INT_LIT);
    assert(tok.int_val == 102);

    tok = next_token(&lex);
    assert(tok.type == TOK_CLOSE_PAR);

    tok = next_token(&lex);
    assert(tok.type == TOK_SEMICOLON);

    tok = next_token(&lex);
    assert(tok.type == TOK_CLOSE_BRACE);

    tok = next_token(&lex);
    assert(tok.type == TOK_EOF);

    free_arena(&ar);
}

void check_lexing(char * text, token_type * types, u64 num_tokens) {
    arena     ar  = make_arena(Mb(8));
    str_store st  = make_str_store(&ar, Mb(2), 101);
    lexer     lex = make_lexer(text, "tst.c", strlen(text), &st);

    u64   i   = 0;
    token tok = {0};
    do {
        tok = next_token(&lex);
        if (tok.type != types[i]) {
            fprintf(stderr,
                    "token %llu: found %s, should be %s\n",
                    i,
                    token_type_to_str(tok.type),
                    token_type_to_str(types[i]));
            exit(1);
        }
        i++;
    } while (tok.type != TOK_EOF && tok.type != TOK_ERR);
    if (i != num_tokens) {
        assert(i < num_tokens);
        fprintf(stderr, "expected %llu tokens, found %llu\n", num_tokens, i);
        exit(1);
    }

    free_arena(&ar);
}

void check_it_does_lex(char * text) {
    arena     ar  = make_arena(Mb(8));
    str_store st  = make_str_store(&ar, Mb(2), 101);
    lexer     lex = make_lexer(text, "tst.c", strlen(text), &st);

    token tok = {0};
    do {
        tok = next_token(&lex);
    } while (tok.type != TOK_EOF && tok.type != TOK_ERR);
    if (tok.type != TOK_EOF) {
        assert(tok.type == TOK_ERR);
        fprintf(stderr, "failed to lex\n");
        exit(1);
    }
}

void test_lexer_1_invalid(void) {
    char * text =
        "int main(void) {\n"
        "return 0@1;\n"
        "}\n";
    token_type types[] = {TOK_INT,
                          TOK_ID,
                          TOK_OPEN_PAR,
                          TOK_VOID,
                          TOK_CLOSE_PAR,
                          TOK_OPEN_BRACE,
                          TOK_RETURN,
                          TOK_INT_LIT,
                          TOK_ERR};
    check_lexing(text, types, array_len(types));

    text             = "\n\\";
    token_type tps[] = {TOK_ERR};
    check_lexing(text, tps, array_len(tps));

    text = "\n`";
    check_lexing(text, tps, array_len(tps));

    text =
        "int main(void) {\n"
        "return 1foo;\n"
        "}\n";
    token_type tps1[] = {TOK_INT,
                         TOK_ID,
                         TOK_OPEN_PAR,
                         TOK_VOID,
                         TOK_CLOSE_PAR,
                         TOK_OPEN_BRACE,
                         TOK_RETURN,
                         TOK_ERR};
    check_lexing(text, tps1, array_len(tps1));

    text =
        "int main(void)\n"
        "{\n"
        "    return @b;\n"
        "}";
    token_type tps2[] = {TOK_INT,
                         TOK_ID,
                         TOK_OPEN_PAR,
                         TOK_VOID,
                         TOK_CLOSE_PAR,
                         TOK_OPEN_BRACE,
                         TOK_RETURN,
                         TOK_ERR};
    check_lexing(text, tps2, array_len(tps2));
}

void test_lexer_1_valid(void) {
    char * text =
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
    check_it_does_lex(text);

    text = "int main(void){return 0;}";
    check_it_does_lex(text);

    check_it_does_lex(
        "int main(void) {\n"
        "    return 0;\n"
        "}\n");

    check_it_does_lex("int   main    (  void)  {   return  0 ; }");
    check_it_does_lex(
        "int	main	(	void)	{	return	0	;	}");
}

#endif  // TESTING
