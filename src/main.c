//#define STB_DS_IMPLEMENTATION
//#define STBDS_REALLOC   arena_realloc
//#define STBDS_FREE      arena_noop

//#define STB_SPRINTF_IMPLEMENTATION 1 
//#include "stb_sprintf.h"

#include "common.h"
#include "mem.h"
#include "str.h"
#include "lex.h"
#include "ast.h"
#include "parse.h"
#include "asm-tree.h"

#include "mem.c"
#include "dynarr.c.gen"
#define TESTING 1
#include "str.c"
#include "lex.c"
#include "ast.c"
#include "parse.c"
#include "asm-tree.c"

int
main(void)
{
    // version 0.1
    test_arena_snprintf();
    test_str_buffer();
    test_str_store();
    test_lexer_1();
    test_lexer_1_invalid();
    test_lexer_1_valid();
    test_ast_1();
    test_parser_1_basic();
    test_parser_1_valid();
    test_parser_1_invalid();
    test_asm_tree();

    return 0;
}
