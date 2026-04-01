#include "common.h"
#include "mem.h"
#include "os.h"

#include "mem.c"
#include "os.c"

int main(int argc, char ** argv) {
    arena  ar  = make_arena(Mb(16));
    FILE * out = fopen("..\\src\\dynarr.c.gen", "w");
    assert(out);
    buffer templ = read_whole_file("..\\src\\dyn_arr_templ.c", &ar);
    assert(templ.contents);

    for (int i = 1; i < argc; i++) {
        char * pos  = NULL;
        char * pos0 = templ.contents;
        fprintf(out, "\n\n\n");
        for (pos  = strstr(pos0, "TYPE"); pos;
             pos0 = pos + 4, pos = strstr(pos + 4, "TYPE")) {
            assert(pos);
            assert(pos0);
            // assert(pos > pos0);
            fwrite(pos0, pos - pos0, 1, out);
            fputs(argv[i], out);
        }
        fwrite(pos0, templ.len - (pos0 - templ.contents), 1, out);
    }

    fclose(out);
    return 0;
}
