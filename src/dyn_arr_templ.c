TYPE_arr
make_TYPE_arr_with_cap(arena *ar, size_t cap)
{
    assert(ar);
    TYPE_arr a = {0};
    a.cap = cap;
    a.v = arena_alloc_type_n(ar, TYPE, cap);
    return a;
}

void
TYPE_arr_push(arena *ar, TYPE_arr *a, TYPE el)
{
    assert(ar);
    assert(a);
    assert(a->len <= a->cap);
    if (a->len == a->cap) {
        size_t new_cap = 1 + 2*a->cap;
        new_cap = maximum(new_cap, 4);
        a->v = arena_alloc_type_n(ar, TYPE, new_cap);
        a->cap = new_cap;
    }
    a->v[a->len++] = el;
}

