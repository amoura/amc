// #define STB_DS_IMPLEMENTATION
// #define STBDS_REALLOC   arena_realloc
// #define STBDS_FREE      arena_noop

// #define STB_SPRINTF_IMPLEMENTATION 1
// #include "stb_sprintf.h"

#include "common.h"
#include "mem.h"
#include "os.h"
#include "str.h"
#include "lex.h"
#include "ast.h"
#include "parse.h"
#include "asm-tree.h"
#include "codegen-x64.h"

#include "dynarr.c.gen"
#include "mem.c"
#include "os.c"
#define TESTING 1
#include "str.c"
#include "lex.c"
#include "ast.c"
#include "parse.c"
#include "asm-tree.c"
#include "codegen-x64.c"

void basic_tests(void);

typedef enum {
    EXEC_MODE_NONE,
    EXEC_MODE_BASIC_TEST,
    EXEC_MODE_LEXER_TEST,
    EXEC_MODE_PARSER_TEST,
    EXEC_MODE_NO_OUTPUT,
    EXEC_MODE_NORMAL,
} exec_mode;

typedef struct {
    exec_mode mode;
    char *    filename;
} cmd_line_data;

void print_usage_and_exit(const char * prog_name, const char * msg) {
    fprintf(stderr, "%s: ERROR: %s\n\n", prog_name, msg);
    fprintf(stderr, "%s: usage:\n", prog_name);
    fprintf(stderr,
            "%s [--basic-test | --lexer-test | --parser-test | --no-output] "
            "<file name>\n",
            prog_name);
    exit(1);
}

cmd_line_data parse_cmd_line(int argc, char ** argv) {
    cmd_line_data result = {0};
    result.mode          = EXEC_MODE_NORMAL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--basic-test") == 0) {
            result.mode = EXEC_MODE_BASIC_TEST;
        } else if (strcmp(argv[i], "--lexer-test") == 0) {
            result.mode = EXEC_MODE_LEXER_TEST;
        } else if (strcmp(argv[i], "--parser-test") == 0) {
            result.mode = EXEC_MODE_PARSER_TEST;
        } else if (strcmp(argv[i], "--no-output") == 0) {
            result.mode = EXEC_MODE_NO_OUTPUT;
        } else {  // must be the filename
            if (result.filename != NULL) {
                print_usage_and_exit(argv[0], "multiple input files");
            }
            result.filename = argv[i];
        }
        if (result.filename == NULL && result.mode != EXEC_MODE_BASIC_TEST) {
            print_usage_and_exit(argv[0], "no input file");
        }
    }
    return result;
}

int main(int argc, char ** argv) {
    basic_tests();

    if (argc < 2) {
        print_usage_and_exit(argv[0], "no input file");
    }
    cmd_line_data cmd_line = parse_cmd_line(argc, argv);

    if (cmd_line.mode == EXEC_MODE_BASIC_TEST) {
        return 0;
    }

    arena  ar  = make_arena(Mb(128));
    buffer buf = read_whole_file(&ar, cmd_line.filename);
    if (buf.contents == NULL) {
        print_usage_and_exit(argv[0], "could not read input file");
    }

    char *    text = buf.contents;
    str_store st   = make_str_store(&ar, Mb(16), 100003);
    lexer     lex  = make_lexer(text, "tst.c", strlen(text), &st);
    if (cmd_line.mode == EXEC_MODE_LEXER_TEST) {
        token tok = {0};
        for (tok = next_token(&lex); tok.type != TOK_EOF && tok.type != TOK_ERR;
             tok = next_token(&lex));
        if (tok.type == TOK_ERR) {
            return 1;
        }
        return 0;
    }

    parser p       = make_parser(text, cmd_line.filename, &st, &ar);
    ast *  program = parse_program(&p);
    assert(program);
    if (program->type == AST_ERR) {
        if (cmd_line.mode == EXEC_MODE_PARSER_TEST) {
            return 1;
        }
        print_parse_error(stderr, &p, program);
        return 1;
    }
    assert(program->type == AST_PROGRAM);
    if (cmd_line.mode == EXEC_MODE_PARSER_TEST) {
        return 0;
    }

    asm_node * node = asm_tree_from_ast(&ar, program);
    assert(node);
    assert(node->type == ASM_NODE_PROGRAM);
    if (cmd_line.mode == EXEC_MODE_NO_OUTPUT) {
        return 0;
    }

    FILE * out = fopen(cmd_line.filename, "w");
    if (out == NULL) {
        fprintf(stderr,
                "error: could not open output file %s\n",
                cmd_line.filename);
        return 1;
    }
    asm_node_codegen_x64(out, node);

    fclose(out);
    return 0;
}

void basic_tests(void) {
    // v0.1
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
    test_codegen_x64_1();
}
