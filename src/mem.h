#ifndef AM_MEM_H
#define AM_MEM_H

#include "common.h"

typedef struct {
    u8 		*memory;
    u64  	cap;
    u64  	pos;
    bool 	is_sub_arena;
} arena;

arena 	make_arena(u64 size);
arena 	make_sub_arena(arena *super_arena, u64 size);
u64 	arena_pos(arena *ar);
void 	arena_pop_to_pos(arena *ar, u64	pos);
void * 	arena_alloc(arena *ar, u64 nbytes);
void 	arena_reset(arena *ar);
void 	free_arena(arena *ar);
char *  arena_sprintf(arena *ar, const char *fmt, ...);

#define arena_alloc_type(a,T)           (T*) arena_alloc(a, sizeof(T))
#define arena_alloc_type_n(a,T,n)       (T*) arena_alloc(a, sizeof(T)*(n))

// use this to define a dynamic array.
// def_dyn_arr(T) defines a dynamic array T_arr.
#define def_dyn_arr(T) \
    typedef struct {\
        size_t len;\
        size_t cap;\
        T *v;\
    } join(T,_arr)

//////////////////////////////////////////////////////////
// For use with am_stb_ds

#if 0
typedef struct {
    size_t size;
    u8 data[0];
} mem_block;

void *
arena_realloc(void *ar_ptr, void *ptr, size_t size);

void
arena_noop(void *ar_ptr, void *ptr);

// If including am_stb_arr, add this before including it
// to make it work with arenas:
//
//   #define STB_DS_IMPLEMENTATION
//   #define STBDS_REALLOC   arena_realloc
//   #define STBDS_FREE      arena_noop

///////////////////////////////////////////////////////////
// Dynamic array macros

#define arr_push(arena,arr,elt) do {\
    assert((arr).len <= (arr).cap);\
    if ((arr).len == (arr).cap) {\
        (arr).cap = 1 + 2*((arr).cap);\
        void *ptr_ = arena_alloc(arena, (arr).cap * sizeof(*((arr).v)));\
        memmove(ptr_, (arr).v, (arr).len * sizeof(*((arr).v)));\
        (arr).v = ptr_;\
    }\
    (arr).v[(arr).len++] = elt;\
} while (false)
#endif

#endif // AM_MEM_H
