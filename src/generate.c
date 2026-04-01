#include "common.h"
#include "mem.h"
#include "os.h"

#include "mem.c"
#include "os.c"
#include <ctype.h>
#include <string.h>

char * skip_whitespace_(char * str) {
    while (*str != 0 && isspace(*str)) {
        str++;
    }
    return str;
}

char * expect_(char * str, const char * expect_ed) {
    size_t len = strlen(expect_ed);
    if (strncmp(str, expect_ed, len) != 0) {
        return NULL;
    }
    return str + len;
}

int main(int num_args, char ** args) {
    arena ar = make_arena(Mb(8));

    for (int file_index = 1; file_index < num_args; file_index++) {
        char * in_file_name      = args[file_index];
        size_t in_file_name_size = strlen(in_file_name);
        char * out_file_name =
            arena_alloc_type_n(&ar, char, in_file_name_size + 5);
        memcpy(out_file_name, in_file_name, in_file_name_size);
        memcpy(out_file_name + in_file_name_size, ".gen", 4);
        assert(strlen(out_file_name) == in_file_name_size + 4);
        assert(strncmp(in_file_name, out_file_name, in_file_name_size) == 0);
        assert(strcmp(out_file_name + in_file_name_size, ".gen") == 0);
        FILE * out = fopen(out_file_name, "w");

        buffer buf  = read_whole_file(in_file_name, &ar);
        char * text = buf.contents;
        for (char * ptr = strstr(text, "\n//gen:enum"); ptr;
             ptr        = strstr(ptr, "\n//gen:enum")) {
            ptr++;
            ptr = strchr(ptr, '\n');
            ptr = skip_whitespace_(ptr);
            ptr = expect_(ptr, "typedef enum ");
            error_if(ptr == NULL, "expect_ed enum");

            ptr         = skip_whitespace_(ptr);
            char * ptr0 = ptr;
            while (*ptr && (isalpha(*ptr) || *ptr == '_')) {
                ptr++;
            }
            error_if(ptr == ptr0, "expect_ed identifier");
            char * enum_name = arena_alloc_type_n(&ar, char, ptr - ptr0 + 1);
            memcpy(enum_name, ptr0, ptr - ptr0);
            assert(strlen(enum_name) == ptr - ptr0);
            // printf("%s: found enum %s\n", in_file_name, enum_name);

            char * fn_name = arena_alloc_type_n(&ar, char, ptr - ptr0 + 8);
            memcpy(fn_name, enum_name, ptr - ptr0);
            memcpy(fn_name + (ptr - ptr0), "_to_str", 7);
            assert(strlen(fn_name) == strlen(enum_name) + 7);

            fprintf(out, "const char *\n");
            fprintf(out, "%s(%s x)\n", fn_name, enum_name);
            fprintf(out, "{\n");
            fprintf(out, "    switch (x) {\n");

            ptr = skip_whitespace_(ptr);
            ptr = expect_(ptr, "{");
            error_if(ptr == NULL, "expect_ed '{'");

            for (ptr = skip_whitespace_(ptr);
                 ptr && *ptr && (isalpha(*ptr) || *ptr == '_');
                 ptr = skip_whitespace_(ptr)) {
                ptr0 = ptr;
                while (*ptr && (isalnum(*ptr) || *ptr == '_')) {
                    ptr++;
                }
                error_if(ptr == ptr0, "expect_ed identifier");
                char * enum_item =
                    arena_alloc_type_n(&ar, char, ptr - ptr0 + 1);
                memcpy(enum_item, ptr0, ptr - ptr0);
                assert(strlen(enum_item) == ptr - ptr0);
                fprintf(out, "    case %s:\n", enum_item);
                fprintf(out, "        return \"%s\";\n", enum_item);
                ptr = expect_(ptr, ",");
                error_if(ptr == NULL, "expect_ed a comma");
            }
            ptr = expect_(ptr, "}");
            error_if(ptr == NULL, "expect_ed '}'");
            ptr = skip_whitespace_(ptr);
            ptr = expect_(ptr, enum_name);
            error_if(ptr == NULL, "expect_ed enum name");
            ptr = skip_whitespace_(ptr);
            ptr = expect_(ptr, ";");

            fprintf(out, "    default:\n");
            fprintf(out, "        assert(false);\n");
            fprintf(out, "    }\n");
            fprintf(out, "    assert(false);\n");
            fprintf(out, "    return 0;\n");
            fprintf(out, "}\n\n");
        }
        fclose(out);
    }
    return 0;
}
