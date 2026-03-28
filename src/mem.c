arena
make_arena(u64	size)
{
    arena ar = {0};
    u64 actual_size = align_up_pow2(size, 8);
    ar.memory = (u8 *) calloc(1, actual_size);
    assert(ar.memory);
    assert(is_aligned_pow2(ar.memory, 8));
    
    ar.cap = actual_size;
    ar.pos = 0;
    ar.is_sub_arena = false;

    return ar;
}

arena
make_sub_arena(arena	*super_ar,
	     u64     	size)
{
    assert(super_ar);
    
    arena sub_ar = {0};
    u64 actual_size = align_up_pow2(size, 8);
    sub_ar.memory = arena_alloc(super_ar, actual_size);
    assert(sub_ar.memory);
    assert(is_aligned_pow2(sub_ar.memory, 8));
    
    sub_ar.cap = actual_size;
    sub_ar.pos = 0;
    sub_ar.is_sub_arena = true;

    assert(super_ar->memory + super_ar->cap >=
	   sub_ar.memory + sub_ar.cap);
    return sub_ar;
}    

void *
arena_alloc(arena	*ar,
	   u64		nbytes)
{
    assert(ar);
    assert(nbytes > 0);
    
    u64 actual_nbytes = align_up_pow2(nbytes, 8);
    assert(actual_nbytes <= (ar->cap - ar->pos)); // TODO: should we return NULL?
    u8 *result = ar->memory + ar->pos;
    assert(is_aligned_pow2(result, 8));
    ar->pos += actual_nbytes;

    assert(ar->pos <= ar->cap);
    return result;
}

u64
arena_pos(arena	*ar)
{
    assert(ar);
    return ar->pos;
}

void
arena_pop_to_pos(arena 	*ar,
	      u64	pos)
{
    assert(ar);
    assert(ar->pos >= pos);
    ar->pos = pos;
}

void
arena_reset(arena	*ar)
{
    assert(ar);
    ar->pos = 0;
}

void
free_arena(arena	*ar)
{
    assert(ar);
    if (! ar->is_sub_arena) {
        free(ar->memory);
    }
    *ar = (arena) {0};
}


///////////////////////////////////////////////
// For integration with am_stb_ds.h

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

void
arena_noop(void *ar_ptr, void *ptr)
{
}


