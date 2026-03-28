#ifndef AM_MEM_H
#define AM_MEM_H

#include "common.h"

typedef struct {
    u8 		*memory;
    u64  	cap;
    u64  	pos;
    bool 	is_sub_arena;
} Arena;

Arena 	MakeArena(u64 		size);
Arena 	MakeSubArena(Arena 	*super_arena,
		     u64	size);
u64 	ArenaPos(Arena 		*arena);
void 	ArenaPopToPos(Arena 	*arena,
		      u64	pos);
void * 	ArenaAlloc(Arena 	*arena,
		   u64 		nbytes);
void 	ArenaReset(Arena 	*arena);
void 	FreeArena(Arena 	*arena);

#define ArenaAllocType(a,T)		(T*) ArenaAlloc(a, sizeof(T))
#define ArenaAllocTypeN(a,T,n)		(T*) ArenaAlloc(a, sizeof(T)*(n))


//////////////////////////////////////////////////////////
// For use wuth am_stb_ds

typedef struct {
    size_t size;
    u8 data[0];
} MemBlock;

void *
ArenaRealloc(void *arena_ptr, void *ptr, size_t size);

void
ArenaNoop(void *arena_ptr, void *ptr);

// If including am_stb_arr, add this before including it
// to make it work with arenas:
//
//   #define STB_DS_IMPLEMENTATION
//   #define STBDS_REALLOC   ArenaRealloc
//   #define STBDS_FREE      ArenaNoop

#if 0
///////////////////////////////////////////////////////////
// Dynamic array macros

#define DefArr(T) \
    typedef struct {\
        size_t len;\
        size_t cap;\
        T *v;\
    } Join(T,Arr)

#define ArrPush(arena,arr,elt) do {\
    assert((arr).len <= (arr).cap);\
    if ((arr).len == (arr).cap) {\
        (arr).cap = 1 + 2*((arr).cap);\
        void *ptr_ = ArenaAlloc(arena, (arr).cap * sizeof(*((arr).v)));\
        memmove(ptr_, (arr).v, (arr).len * sizeof(*((arr).v)));\
        (arr).v = ptr_;\
    }\
    (arr).v[(arr).len++] = elt;\
} while (false)
#endif

#endif // AM_MEM_H
