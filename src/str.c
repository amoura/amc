str_buffer
make_str_buffer(arena *ar, u64 size)
{
    str_buffer str_buf = {0};
    str_buf.buffer = arena_alloc_type_n(ar, char, size);
    str_buf.pos = 0;
    str_buf.size = size;
    return str_buf;
}

bool
str_fits_in_buffer(str_buffer *strb, u64 len)
{
    return len+1+strb->pos <= strb->size;
}

char *
append_str_to_buffer(str_buffer *strb, char *str, u64 len)
{
    error_if(!str_fits_in_buffer(strb, len), 
            "insufficient space in string storate.");
    char *result = strb->buffer+strb->pos;
    memmove(result, str, len);
    strb->pos += len+1;
    assert(strb->pos <= strb->size);
    return result;
}

u64
hash_str(char *str, u64 len)
{
    u64 hash = 0;
    for_i(u64, i, len) {
        hash = hash*31 + (u64)(str[i]);
    }
    return hash;
}

str_store
make_str_store(arena *ar, u64 buffer_size, u64 num_buckets)
{
    str_store st = {0};
    st.strs = make_str_buffer(ar, buffer_size);
    st.table = arena_alloc_type_n(ar, str_node*, num_buckets);
    st.num_buckets = num_buckets;
    st.ar = make_sub_arena(ar, buffer_size);
    return st;
}

bool
str_eq_n(char *s1, char *s2, u64 len)
{
    return strncmp(s1,s2,len) == 0;
}

bool
str_eq(char *s1, char *s2)
{
    return strcmp(s1,s2) == 0;
}


char *
intern_str_n(str_store *st, char *str, u64 len)
{
    u64 bucket_index = hash_str(str, len) % st->num_buckets;
    str_node *bucket = st->table[bucket_index];
    for(str_node *node=bucket; node; node=node->next) {
        assert(node->str);
        assert(st->strs.pos + len < st->strs.size);
        if (str_eq_n(node->str,str,len) && node->str[len]==0) {
            // Found a sting in store, return its address.
            return node->str;
        }
    }

    // Did not find a string.
    char *new_str = append_str_to_buffer(&st->strs, str, len);
    assert(new_str);
    str_node *new_node = arena_alloc_type(&st->ar, str_node);
    new_node->str = new_str;
    new_node->next = bucket;
    st->table[bucket_index] = new_node;
    return new_str;
}

char *
intern_str(str_store *st, char *str)
{
    u64 len = strlen(str);
    return intern_str_n(st, str, len);
}

char *
find_stored_str_n(str_store *st, char *str, u64 len)
{
    u64 bucket_index = hash_str(str, len) % st->num_buckets;
    str_node *bucket = st->table[bucket_index];
    for(str_node *node=bucket; node; node=node->next) {
        assert(node->str);
        assert(st->strs.pos + len < st->strs.size);
        if (str_eq_n(node->str,str,len) && node->str[len]==0) {
            // Found a sting in store, return its address.
            return node->str;
        }
    }
    return NULL;
}

char *
find_stored_str(str_store *st, char *str)
{
    u64 len = strlen(str);
    return find_stored_str_n(st, str, len);
}


////////////////////////////////////////////////////////
// Tests

#ifdef TESTING

void 
test_str_buffer(void)
{
    arena ar = make_arena(Mb(2));
    str_buffer strb = make_str_buffer(&ar, Mb(1));

    char w1[] = "Apple";
    char w2[] = "Banana";
    char w3[] = "Pear";

    char *p1 = append_str_to_buffer(&strb, w1, strlen(w1));
    char *p2 = append_str_to_buffer(&strb, w2, strlen(w2));
    char *p3 = append_str_to_buffer(&strb, w3, strlen(w3));

    assert(p1);
    assert(strcmp(p1,w1) == 0);
    assert(p2);
    assert(strcmp(p2,w2) == 0);
    assert(p3);
    assert(strcmp(p3,w3) == 0);

    free_arena(&ar);
}

void 
test_str_store(void)
{
    arena ar = make_arena(Mb(4));
    str_store st = make_str_store(&ar, Mb(1), 5);

    char *s1 = intern_str_n(&st, "apple", 5);
    char *s2 = intern_str_n(&st, "orange", 6);
    char *s1r = intern_str_n(&st, "apple", 5);
    char *s3 = intern_str_n(&st, "grape", 5);
    char *s4 = intern_str_n(&st, "avocado", 7);
    char *s5 = intern_str_n(&st, "pear", 4);
    char *s6 = intern_str_n(&st, "banana", 6);
    char *s7 = intern_str_n(&st, "acai", 4);
    char *s8 = intern_str_n(&st, "grapefruit", 10);
    char *s5r = intern_str_n(&st, "pear", 4);
    char *s9 = intern_str_n(&st, "lemon juice", 5);
    char *sa = intern_str_n(&st, "tamarind", 8);
    char *sb = intern_str_n(&st, "ata", 3);
    char *s1rr = intern_str_n(&st, "apple", 5);
    char *sar = intern_str_n(&st, "tamarind", 8);

    assert(s1); 
    assert(str_eq(s1, "apple"));
    assert(s2); assert(str_eq(s2, "orange"));
    assert(s1r); assert(str_eq(s1r, "apple"));
    assert(s1r == s1);
    assert(s3); assert(str_eq(s3, "grape"));
    assert(s4); assert(str_eq(s4, "avocado"));
    assert(s5); assert(str_eq(s5, "pear"));
    assert(s6); assert(str_eq(s6, "banana"));
    assert(s7); assert(str_eq(s7, "acai"));
    assert(s8); assert(str_eq(s8, "grapefruit"));
    assert(s5r); assert(str_eq(s5r, "pear"));
    assert(s5r == s5);
    assert(s9); assert(str_eq(s9, "lemon"));
    assert(sa); assert(str_eq(sa, "tamarind"));
    assert(sb); assert(str_eq(sb, "ata"));
    assert(s1rr); assert(str_eq(s1rr, "apple"));
    assert(s1rr == s1);
    assert(sar); assert(str_eq(sar, "tamarind"));
    assert(sar == sa);

    assert(find_stored_str_n(&st, "apple", 5) == s1);
    assert(find_stored_str_n(&st, "grape", 5) == s3);
    assert(find_stored_str_n(&st, "banana", 6) == s6);
    assert(find_stored_str_n(&st, "ata", 3) == sb);
    assert(find_stored_str_n(&st, "lemon", 5) == s9);
    assert(find_stored_str_n(&st, "joy", 3) == NULL);
    assert(find_stored_str_n(&st, "apples", 6) == NULL);
    assert(find_stored_str_n(&st, "app", 3) == NULL);
    assert(find_stored_str_n(&st, "tamarin", 7) == NULL);

    //for_i(u64,i,st.num_buckets) {
    //    for (str_node *node=st.table[i]; node; node=node->next) {
    //        printf("%s ", node->str);
    //    }
    //    printf("\n");
    //}

    free_arena(&ar);
}
 

#endif // TESTING
