#define STB_DS_IMPLEMENTATION
#define STBDS_REALLOC   arena_realloc
#define STBDS_FREE      arena_noop

#include "common.h"
#include "mem.h"
#include "am_stb_ds.h"
#include "str.h"
#include "lex.h"
#include "ast.h"
#include "parse.h"

#include "mem.c"
#define TESTING 1
#include "str.c"
#include "lex.c"
#include "ast.c"
#include "parse.c"


int
main(void)
{
    // version 0.1
    test_str_buffer();
    test_str_store();
    test_lexer_1();
    test_lexer_1_invalid();
    test_lexer_1_valid();
    test_ast_1();
    test_parser_1_basic();
    test_parser_1_valid();
    test_parser_1_invalid();

    return 0;
}
