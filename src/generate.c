#include <ctype.h>
#include <string.h>

#include "common.h"
#include "mem.h"
#include "os.h"

#include "mem.c"
#include "os.c"

char *
SkipWhitespace(char *str)
{
    while (*str!=0 && isspace(*str)) {
        str++;
    }
    return str;
}

char *
Expect(char *str, const char *expected)
{
    size_t len = strlen(expected);
    if (strncmp(str,expected,len) != 0) {
        return NULL;
    }
    return str + len;
}

int main(int num_args, char **args)
{
    Arena arena = MakeArena(Mb(8));

    for (int file_index=1; file_index<num_args; file_index++) {
        char *in_file_name = args[file_index];
        size_t in_file_name_size = strlen(in_file_name);
        char *out_file_name = ArenaAllocTypeN(&arena, char, in_file_name_size+5);
        memcpy(out_file_name, in_file_name, in_file_name_size);
        memcpy(out_file_name+in_file_name_size, ".gen", 4);
        assert(strlen(out_file_name) == in_file_name_size + 4);
        assert(strncmp(in_file_name, out_file_name, in_file_name_size) == 0);
        assert(strcmp(out_file_name+in_file_name_size, ".gen") == 0);
        FILE *out = fopen(out_file_name, "w");

        Buffer buffer = ReadWholeFile(in_file_name, &arena);
        char *text = buffer.contents;
        for (char *ptr=strstr(text, "\n//gen:enum");
             ptr;
             ptr=strstr(ptr, "\n//gen:enum")) 
        {
            ptr++;
            ptr = strchr(ptr, '\n');
            ptr = SkipWhitespace(ptr);
            ptr = Expect(ptr, "typedef enum ");
            ErrorIf(ptr == NULL, "expected enum");
            
            ptr = SkipWhitespace(ptr);
            char *ptr0 = ptr;
            while (*ptr && isalpha(*ptr)) {
                ptr++;
            }
            ErrorIf(ptr == ptr0, "expected identifier");
            char *enum_name = ArenaAllocTypeN(&arena, char, ptr-ptr0+1);
            memcpy(enum_name, ptr0, ptr-ptr0);
            assert(strlen(enum_name) == ptr-ptr0);
            printf("%s: found enum %s\n", in_file_name, enum_name);
            
            char *fn_name = ArenaAllocTypeN(&arena, char, ptr-ptr0+6);
            memcpy(fn_name, enum_name, ptr-ptr0);
            memcpy(fn_name+(ptr-ptr0), "ToStr", 5);
            assert(strlen(fn_name) == strlen(enum_name) + 5);

            fprintf(out, "const char *\n");
            fprintf(out, "%s(%s x)\n", fn_name, enum_name);
            fprintf(out, "{\n");
            fprintf(out, "    switch (x) {\n");

            ptr = SkipWhitespace(ptr);
            ptr = Expect(ptr, "{");
            ErrorIf(ptr==NULL, "expected '{'");

            for (ptr = SkipWhitespace(ptr);
                 ptr && *ptr && (isalpha(*ptr) || *ptr=='_');
                 ptr = SkipWhitespace(ptr))
            {
                ptr0 = ptr;
                while (*ptr && (isalpha(*ptr) || *ptr=='_')) {
                    ptr++;
                }
                ErrorIf(ptr == ptr0, "expected identifier");
                char *enum_item = ArenaAllocTypeN(&arena, char, ptr-ptr0+1);
                memcpy(enum_item, ptr0, ptr-ptr0);
                assert(strlen(enum_item) == ptr-ptr0);
                fprintf(out, "    case %s:\n", enum_item);
                fprintf(out, "        return \"%s\";\n", enum_item);
                ptr = Expect(ptr, ",");
                ErrorIf(ptr==NULL, "expected a comma");
            }
            ptr = Expect(ptr, "}");
            ErrorIf(ptr==NULL, "expected '}'");
            ptr = SkipWhitespace(ptr);
            ptr = Expect(ptr, enum_name);
            ErrorIf(ptr==NULL, "expected enum name");
            ptr = SkipWhitespace(ptr);
            ptr = Expect(ptr, ";");

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



                


