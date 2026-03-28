typedef struct {
    char *buffer;
    u64 pos;
    u64 size;
} str_buffer;

typedef struct str_node {
    struct str_node *next;
    char *str;
} str_node;

typedef struct {
    str_buffer strs;
    str_node **table;
    u64 num_buckets;
    arena ar;
} str_store;


str_buffer
make_str_buffer(arena *ar, u64 size);

bool
str_fits_in_buffer(str_buffer *strb, u64 len);

char *
append_str_to_buffer(str_buffer *strb, char *str, u64 len);

str_store
make_str_store(arena *ar, u64 buffer_size, u64 num_buckets);

char *
intern_str_n(str_store *st, char *str, u64 len);

char *
intern_str(str_store *st, char *str);

char *
find_stored_str_n(str_store *st, char *str, u64 len);

char *
find_stored_str(str_store *st, char *str);
