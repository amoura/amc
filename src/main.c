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
#include "ir.h"
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
#include "ir.c"
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
    bool      driver;
    char *    in_filename;
    char *    out_filename;
} cmd_line_data;

void print_usage_and_exit(const char * prog_name, const char * msg) {
    fprintf(stderr, "%s: ERROR: %s\n\n", prog_name, msg);
    fprintf(stderr, "%s: usage:\n", prog_name);
    fprintf(stderr,
            "%s [--basic-test | --lexer-test | --parser-test | --no-output] \n"
            "    -o <output-file-name> <input-file-name>\n",
            prog_name);
    exit(1);
}

void error_and_exit_if(bool cond, char * prog_name, char * msg) {
    if (!cond) {
        return;
    }
    fprintf(stderr, "%s: ERROR: %s\n", prog_name, msg);
    exit(1);
}

void warn_if(bool cond, char * prog_name, char * msg) {
    if (!cond) {
        return;
    }
    fprintf(stderr, "%s: warning: %s\n", prog_name, msg);
    exit(1);
}

char * program_name(char * arg) {
    size_t len = strlen(arg);
    assert(len > 0);
    char * p = arg + len - 1;
    for (; p >= arg && *p != '/' && *p != '\\'; p--);
    return p + 1;
}

cmd_line_data parse_cmd_line(int argc, char ** argv) {
    cmd_line_data result = {0};
    result.mode          = EXEC_MODE_NORMAL;
    char * prog_name     = program_name(argv[0]);
    if (str_eq(prog_name, "amcc")) {
        result.driver = true;
    } else if (str_eq(prog_name, "amc")) {
        result.driver = false;
    } else {
        print_usage_and_exit(prog_name, "I don't know who I am!");
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0) {
            i++;
            assert(i <= argc);
            if (i == argc) {
                print_usage_and_exit(prog_name, "missing output file name");
            }
            result.out_filename = argv[i];
        } else if (strcmp(argv[i], "--basic-test") == 0) {
            result.mode = EXEC_MODE_BASIC_TEST;
        } else if (strcmp(argv[i], "--lexer-test") == 0) {
            result.mode = EXEC_MODE_LEXER_TEST;
        } else if (strcmp(argv[i], "--parser-test") == 0) {
            result.mode = EXEC_MODE_PARSER_TEST;
        } else if (strcmp(argv[i], "--no-output") == 0) {
            result.mode = EXEC_MODE_NO_OUTPUT;
        } else {  // must be the filename
            if (result.in_filename != NULL) {
                print_usage_and_exit(prog_name, "multiple input files");
            }
            result.in_filename = argv[i];
        }
    }
    if (result.in_filename == NULL && result.mode != EXEC_MODE_BASIC_TEST) {
        print_usage_and_exit(prog_name, "no input file");
    }
    if (result.out_filename == NULL && result.mode == EXEC_MODE_NORMAL) {
        print_usage_and_exit(prog_name, "no output file");
    }
    return result;
}

int main(int argc, char ** argv) {
    char *        prog_name = argv[0];
    cmd_line_data cmd_line  = parse_cmd_line(argc, argv);

    basic_tests();
    if (cmd_line.mode == EXEC_MODE_BASIC_TEST) {
        return 0;
    }

    if (argc < 2) {
        print_usage_and_exit(argv[0], "");
    }

    arena  ar  = make_arena(Mb(128));
    buffer buf = {0};
    if (cmd_line.driver) {
        char * pp_out_fname = arena_sprintf(&ar, "%s.i", cmd_line.in_filename);
        char * cmd          = arena_sprintf(&ar,
                                            "gcc -E -P %s -o %s",
                                            cmd_line.in_filename,
                                            pp_out_fname);
        int    ret_code     = system(cmd);
        error_and_exit_if(ret_code != 0,
                          prog_name,
                          "failed to call preprocessor");
        buf = read_whole_file(&ar, pp_out_fname);
        error_and_exit_if(!buf.contents,
                          prog_name,
                          "failed to read preprocessor's output");
        ret_code = remove(pp_out_fname);
        warn_if(ret_code != 0,
                prog_name,
                "could not delete proprocessor's output");
    } else {
        buf = read_whole_file(&ar, cmd_line.in_filename);
        error_and_exit_if(!buf.contents, prog_name, "failed to read input");
    }

    char *    text = buf.contents;
    str_store st   = make_str_store(&ar, Mb(16), 100003);
    lexer     lex  = make_lexer(text, cmd_line.in_filename, strlen(text), &st);
    if (cmd_line.mode == EXEC_MODE_LEXER_TEST) {
        token tok = {0};
        for (tok = next_token(&lex); tok.type != TOK_EOF && tok.type != TOK_ERR;
             tok = next_token(&lex));
        if (tok.type == TOK_ERR) {
            return 1;
        }
        return 0;
    }

    parser p       = make_parser(text, cmd_line.in_filename, &st, &ar);
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

    if (cmd_line.driver) {
        char * asm_out_fname = arena_sprintf(&ar, "%s.s", cmd_line.in_filename);

        FILE * out = fopen(asm_out_fname, "w");
        error_and_exit_if(out == NULL, prog_name, "could not open output file");
        asm_node_codegen_x64(out, node);
        fclose(out);

        char * cmd      = arena_sprintf(&ar,
                                        "gcc %s -o %s",
                                        asm_out_fname,
                                        cmd_line.out_filename);
        int    ret_code = system(cmd);
        error_and_exit_if(ret_code != 0, prog_name, "failed to call assembler");

        ret_code = remove(asm_out_fname);
        warn_if(ret_code != 0,
                prog_name,
                "could not delete assembler's output");
    } else {
        FILE * out = fopen(cmd_line.out_filename, "w");
        error_and_exit_if(out == NULL, prog_name, "could not open output file");
        asm_node_codegen_x64(out, node);
        fclose(out);
    }

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
