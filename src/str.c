StrBuffer
MakeStrBuffer(Arena *arena, u64 size)
{
    StrBuffer str_buffer = {0};
    str_buffer.buffer = ArenaAllocTypeN(arena, char, size);
    str_buffer.pos = 0;
    str_buffer.size = size;
    return str_buffer;
}

bool
StrFitsInBuffer(StrBuffer *strb, u64 len)
{
    return len+1+strb->pos <= strb->size;
}

char *
AppendStrToBuffer(StrBuffer *strb, char *str, u64 len)
{
    ErrorIf(!StrFitsInBuffer(strb, len), 
            "insufficient space in string storate.");
    char *result = strb->buffer+strb->pos;
    memmove(result, str, len);
    strb->pos += len+1;
    assert(strb->pos <= strb->size);
    return result;
}

u64
HashStr(char *str, u64 len)
{
    u64 hash = 0;
    for_i(u64, i, len) {
        hash = hash*31 + (u64)(str[i]);
    }
    return hash;
}

StrStore
MakeStrStore(Arena *arena, u64 buffer_size, u64 num_buckets)
{
    StrStore st = {0};
    st.strs = MakeStrBuffer(arena, buffer_size);
    st.table = ArenaAllocTypeN(arena, StrNode*, num_buckets);
    st.num_buckets = num_buckets;
    st.arena = MakeSubArena(arena, buffer_size);
    return st;
}

bool
StrEqN(char *s1, char *s2, u64 len)
{
    return strncmp(s1,s2,len) == 0;
}

bool
StrEq(char *s1, char *s2)
{
    return strcmp(s1,s2) == 0;
}


char *
InternStrN(StrStore *st, char *str, u64 len)
{
    u64 bucket_index = HashStr(str, len) % st->num_buckets;
    StrNode *bucket = st->table[bucket_index];
    for(StrNode *node=bucket; node; node=node->next) {
        assert(node->str);
        assert(st->strs.pos + len < st->strs.size);
        if (StrEqN(node->str,str,len) && node->str[len]==0) {
            // Found a sting in store, return its address.
            return node->str;
        }
    }

    // Did not find a string.
    char *new_str = AppendStrToBuffer(&st->strs, str, len);
    assert(new_str);
    StrNode *new_node = ArenaAllocType(&st->arena, StrNode);
    new_node->str = new_str;
    new_node->next = bucket;
    st->table[bucket_index] = new_node;
    return new_str;
}

char *
InternStr(StrStore *st, char *str)
{
    u64 len = strlen(str);
    return InternStrN(st, str, len);
}

char *
FindStoredStrN(StrStore *st, char *str, u64 len)
{
    u64 bucket_index = HashStr(str, len) % st->num_buckets;
    StrNode *bucket = st->table[bucket_index];
    for(StrNode *node=bucket; node; node=node->next) {
        assert(node->str);
        assert(st->strs.pos + len < st->strs.size);
        if (StrEqN(node->str,str,len) && node->str[len]==0) {
            // Found a sting in store, return its address.
            return node->str;
        }
    }
    return NULL;
}

char *
FindStoredStr(StrStore *st, char *str)
{
    u64 len = strlen(str);
    return FindStoredStrN(st, str, len);
}


////////////////////////////////////////////////////////
// Tests

#ifdef TESTING

void 
test_str_buffer(void)
{
    Arena arena = MakeArena(Mb(2));
    StrBuffer strb = MakeStrBuffer(&arena, Mb(1));

    char w1[] = "Apple";
    char w2[] = "Banana";
    char w3[] = "Pear";

    char *p1 = AppendStrToBuffer(&strb, w1, strlen(w1));
    char *p2 = AppendStrToBuffer(&strb, w2, strlen(w2));
    char *p3 = AppendStrToBuffer(&strb, w3, strlen(w3));

    assert(p1);
    assert(strcmp(p1,w1) == 0);
    assert(p2);
    assert(strcmp(p2,w2) == 0);
    assert(p3);
    assert(strcmp(p3,w3) == 0);

    FreeArena(&arena);
}

void 
test_str_store(void)
{
    Arena arena = MakeArena(Mb(4));
    StrStore st = MakeStrStore(&arena, Mb(1), 5);

    char *s1 = InternStrN(&st, "apple", 5);
    char *s2 = InternStrN(&st, "orange", 6);
    char *s1r = InternStrN(&st, "apple", 5);
    char *s3 = InternStrN(&st, "grape", 5);
    char *s4 = InternStrN(&st, "avocado", 7);
    char *s5 = InternStrN(&st, "pear", 4);
    char *s6 = InternStrN(&st, "banana", 6);
    char *s7 = InternStrN(&st, "acai", 4);
    char *s8 = InternStrN(&st, "grapefruit", 10);
    char *s5r = InternStrN(&st, "pear", 4);
    char *s9 = InternStrN(&st, "lemon juice", 5);
    char *sa = InternStrN(&st, "tamarind", 8);
    char *sb = InternStrN(&st, "ata", 3);
    char *s1rr = InternStrN(&st, "apple", 5);
    char *sar = InternStrN(&st, "tamarind", 8);

    assert(s1); 
    assert(StrEq(s1, "apple"));
    assert(s2); assert(StrEq(s2, "orange"));
    assert(s1r); assert(StrEq(s1r, "apple"));
    assert(s1r == s1);
    assert(s3); assert(StrEq(s3, "grape"));
    assert(s4); assert(StrEq(s4, "avocado"));
    assert(s5); assert(StrEq(s5, "pear"));
    assert(s6); assert(StrEq(s6, "banana"));
    assert(s7); assert(StrEq(s7, "acai"));
    assert(s8); assert(StrEq(s8, "grapefruit"));
    assert(s5r); assert(StrEq(s5r, "pear"));
    assert(s5r == s5);
    assert(s9); assert(StrEq(s9, "lemon"));
    assert(sa); assert(StrEq(sa, "tamarind"));
    assert(sb); assert(StrEq(sb, "ata"));
    assert(s1rr); assert(StrEq(s1rr, "apple"));
    assert(s1rr == s1);
    assert(sar); assert(StrEq(sar, "tamarind"));
    assert(sar == sa);

    assert(FindStoredStrN(&st, "apple", 5) == s1);
    assert(FindStoredStrN(&st, "grape", 5) == s3);
    assert(FindStoredStrN(&st, "banana", 6) == s6);
    assert(FindStoredStrN(&st, "ata", 3) == sb);
    assert(FindStoredStrN(&st, "lemon", 5) == s9);
    assert(FindStoredStrN(&st, "joy", 3) == NULL);
    assert(FindStoredStrN(&st, "apples", 6) == NULL);
    assert(FindStoredStrN(&st, "app", 3) == NULL);
    assert(FindStoredStrN(&st, "tamarin", 7) == NULL);

    //for_i(u64,i,st.num_buckets) {
    //    for (StrNode *node=st.table[i]; node; node=node->next) {
    //        printf("%s ", node->str);
    //    }
    //    printf("\n");
    //}

    FreeArena(&arena);
}
 

#endif // TESTING
