#include "common.h"
#include "mem.h"

Arena
MakeArena(u64	size)
{
    Arena arena = {0};
    u64 actual_size = AlignUpPow2(size, 8);
    arena.memory = (u8 *) calloc(1, actual_size);
    assert(arena.memory);
    assert(IsAlignedPow2(arena.memory, 8));
    
    arena.cap = actual_size;
    arena.pos = 0;
    arena.is_sub_arena = false;

    return arena;
}

Arena
MakeSubArena(Arena	*super_arena,
	     u64     	size)
{
    assert(super_arena);
    
    Arena sub_arena = {0};
    u64 actual_size = AlignUpPow2(size, 8);
    sub_arena.memory = ArenaAlloc(super_arena, actual_size);
    assert(sub_arena.memory);
    assert(IsAlignedPow2(sub_arena.memory, 8));
    
    sub_arena.cap = actual_size;
    sub_arena.pos = 0;
    sub_arena.is_sub_arena = true;

    assert(super_arena->memory + super_arena->cap >=
	   sub_arena.memory + sub_arena.cap);
    return sub_arena;
}    

void *
ArenaAlloc(Arena	*arena,
	   u64		nbytes)
{
    assert(arena);
    assert(nbytes > 0);
    
    u64 actual_nbytes = AlignUpPow2(nbytes, 8);
    assert(actual_nbytes <= (arena->cap - arena->pos)); // TODO: should we return NULL?
    u8 *result = arena->memory + arena->pos;
    assert(IsAlignedPow2(result, 8));
    arena->pos += actual_nbytes;

    assert(arena->pos <= arena->cap);
    return result;
}

u64
ArenaPos(Arena	*arena)
{
    assert(arena);
    return arena->pos;
}

void
ArenaPopToPos(Arena 	*arena,
	      u64	pos)
{
    assert(arena);
    assert(arena->pos >= pos);
    arena->pos = pos;
}

void
ArenaReset(Arena	*arena)
{
    assert(arena);
    arena->pos = 0;
}

void
FreeArena(Arena	*arena)
{
    assert(arena);
    if (! arena->is_sub_arena) {
	free(arena->memory);
    }
    *arena = (Arena) {0};
}


///////////////////////////////////////////////
// For integration with am_stb_ds.h

void *
ArenaRealloc(void *arena_ptr, void *ptr, size_t size)
{
    Arena *arena = (Arena *) arena_ptr;
    if (ptr == NULL) {
        MemBlock *block = ArenaAlloc(arena, sizeof(MemBlock) + size);
        block->size = size;
        return block->data;
    }
    MemBlock *old_block = (MemBlock *)(ptr - OffsetOfMember(MemBlock,data));
    size_t old_size = old_block->size;
    MemBlock *block = ArenaAlloc(arena, sizeof(MemBlock) + size);
    block->size = size;
    memmove(block->data, old_block->data, old_size);
    return block->data;
}

void
ArenaNoop(void *arena_ptr, void *ptr)
{
}


