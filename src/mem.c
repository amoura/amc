arena make_arena(u64 size) {
    arena ar          = {0};
    u64   actual_size = align_up_pow2(size, 8);
    ar.memory         = (u8 *)calloc(1, actual_size);
    assert(ar.memory);
    assert(is_aligned_pow2(ar.memory, 8));

    ar.cap          = actual_size;
    ar.pos          = 0;
    ar.is_sub_arena = false;

    return ar;
}

arena make_sub_arena(arena * super_ar, u64 size) {
    assert(super_ar);

    arena sub_ar      = {0};
    u64   actual_size = align_up_pow2(size, 8);
    sub_ar.memory     = arena_alloc(super_ar, actual_size);
    assert(sub_ar.memory);
    assert(is_aligned_pow2(sub_ar.memory, 8));

    sub_ar.cap          = actual_size;
    sub_ar.pos          = 0;
    sub_ar.is_sub_arena = true;

    assert(super_ar->memory + super_ar->cap >= sub_ar.memory + sub_ar.cap);
    return sub_ar;
}

void * arena_alloc(arena * ar, u64 nbytes) {
    assert(ar);
    assert(nbytes > 0);

    u64 actual_nbytes = align_up_pow2(nbytes, 8);
    assert(actual_nbytes <=
           (ar->cap - ar->pos));  // TODO: should we return NULL?
    u8 * result = ar->memory + ar->pos;
    assert(is_aligned_pow2(result, 8));
    ar->pos += actual_nbytes;

    assert(ar->pos <= ar->cap);
    return result;
}

u64 arena_pos(arena * ar) {
    assert(ar);
    return ar->pos;
}

void arena_pop_to_pos(arena * ar, u64 pos) {
    assert(ar);
    assert(ar->pos >= pos);
    ar->pos = pos;
}

void arena_reset(arena * ar) {
    assert(ar);
    ar->pos = 0;
}

void free_arena(arena * ar) {
    assert(ar);
    if (!ar->is_sub_arena) {
        free(ar->memory);
    }
    *ar = (arena){0};
}

//////////////////////////////////////////////
// arena strung utilities

bool str_eq_n(char * s1, char * s2, u64 len) {
    return strncmp(s1, s2, len) == 0;
}

bool str_eq(char * s1, char * s2) {
    return strcmp(s1, s2) == 0;
}

char * arena_strdup(arena * ar, char * s) {
    size_t len = strlen(s);
    char * res = arena_alloc_type_n(ar, char, len + 1);
    memcpy(res, s, len);
    res[len] = 0;
    return res;
}

char * arena_sprintf(arena * ar, const char * fmt, ...) {
    va_list args;
    va_start(args, fmt);

    u64    pos0   = arena_pos(ar);
    char * dst    = arena_alloc_type_n(ar, char, 8);
    int    nbytes = vsnprintf(dst, 7, fmt, args);
    assert(nbytes > 0);
    if (nbytes > 6) {
        arena_pop_to_pos(ar, pos0);
        dst = arena_alloc_type_n(ar, char, nbytes + 1);
        va_start(args, fmt);
        int nb = vsnprintf(dst, nbytes, fmt, args);
        assert(nb == nbytes);
    }
    va_end(args);
    return dst;
}

///////////////////////////////////////////////
// For integration with am_stb_ds.h

/*

void *
arena_realloc(void *ar_ptr, void *ptr, size_t size)
{
    arena *ar = (arena *) ar_ptr;
    if (ptr == NULL) {
        mem_block *block = arena_alloc(ar, sizeof(mem_block) + size);
        block->size = size;
        return block->data;
    }
    mem_block *old_block = (mem_block *)((char *)ptr - offset_of_member(mem_block,data));
    size_t old_size = old_block->size;
    mem_block *block = arena_alloc(ar, sizeof(mem_block) + size);
    block->size = size;
    memmove(block->data, old_block->data, old_size);
    return block->data;
}
C:\Program Files (x86)\Windows Kits\10\include\10.0.26100.0\um\minwinbase.h
void
arena_noop(void *ar_ptr, void *ptr)
{
}

*/

/////////////////////////////////////////////////
// Tests

void test_arena_snprintf(void) {
    arena  ar  = make_arena(Mb(1));
    double a   = 23.7899;
    double b   = 1299775.877;
    double c   = a * b;
    char * msg = arena_sprintf(&ar,
                               "this is a %s: testing %lg * %lg = %lg\n",
                               "big test",
                               a,
                               b,
                               c);
    assert(msg);
    // printf("%s", msg);

    free_arena(&ar);
}
