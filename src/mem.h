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

#endif // AM_MEM_H
