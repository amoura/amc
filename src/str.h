typedef struct {
    char *buffer;
    u64 pos;
    u64 size;
} StrBuffer;

typedef struct StrNode {
    struct StrNode *next;
    char *str;
} StrNode;

typedef struct {
    StrBuffer strs;
    StrNode **table;
    u64 num_buckets;
    Arena arena;
} StrStore;


StrBuffer
MakeStrBuffer(Arena *arena, u64 size);

bool
StrFitsInBuffer(StrBuffer *strb, u64 len);

char *
AppendStrToBuffer(StrBuffer *strb, char *str, u64 len);

StrStore
MakeStrStore(Arena *arena, u64 buffer_size, u64 num_buckets);

char *
InternStrN(StrStore *st, char *str, u64 len);

char *
InternStr(StrStore *st, char *str);

char *
FindStoredStrN(StrStore *st, char *str, u64 len);

char *
FindStoredStr(StrStore *st, char *str);
