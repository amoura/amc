#include <stdio.h>
#include "common.h"
#include "mem.h"
#include "os.h"

#if defined(OS_LINUX) || defined(OS_MAC)
#include "os-linux.c"
#elif defined(OS_WINDOWS)
#include "os-windows.c"
#elif
#error Unknown OS
#endif

////////////////////////////////////////////////////////////
// File I/O

buffer read_whole_file(arena * ar, const char * filename) {
    FILE * fp      = NULL;
    long   bufsize = 0;
    char * source  = NULL;
    buffer buf     = {0};

    fp = fopen(filename, "rb");
    if (!fp) {
        perror("Error opening file for reading");
        return buf;
    }

    int fseek_ret = fseek(fp, 0L, SEEK_END);

    bufsize = ftell(fp);
    if (bufsize == -1) {
        perror("Error getting file size");
        fclose(fp);
        return buf;
    }

    if (fseek(fp, 0L, SEEK_SET) != 0) {
        perror("Error seeking to start of file");
        fclose(fp);
        return buf;
    }

    source      = arena_alloc_type_n(ar, char, bufsize + 1);
    u64    mark = arena_pos(ar);
    size_t len  = fread(source, sizeof(char), bufsize, fp);
    if (ferror(fp) != 0) {
        fputs("Error reading file", stderr);
        arena_pop_to_pos(ar, mark);
        fclose(fp);
        return buf;
    } else {
        source[len] = 0;
    }

    // Close the file
    fclose(fp);

    buf.len      = len;
    buf.contents = source;
    return buf;
}

////////////////////////////////////////////////////////////
// Multi-thring

simple_thread_pool create_simple_thread_pool(simple_thread_pool_fn * worker_fn,
                                             void **                 data,
                                             u64                     num_tasks,
                                             u64                     num_thrs,
                                             u64     mem_per_thr,
                                             arena * ar) {
    simple_thread_pool pool = {0};
    assert(num_thrs <= array_len(pool.thrs));

    pool.worker_fn       = worker_fn;
    pool.data            = data;
    pool.num_tasks       = num_tasks;
    pool.num_thrs        = num_thrs;
    pool.next_task_index = 0;
    pool.mut             = make_mutex();
    pool.ar              = ar;
    pool.mem_per_thr     = mem_per_thr;

    return pool;
}

void destroy_simple_thread_pool(simple_thread_pool * pool) {
    for (u64 i = 0; i < pool->num_thrs; i++) {
        close_thread(pool->thrs[i]);
    }
    destroy_mutex(&pool->mut);
}

void * simple_thread_pool_get_next_task(simple_thread_pool * pool) {
    lock_mutex(&pool->mut);
    assert(pool->next_task_index < pool->num_tasks + 1);
    void * task_data = NULL;
    if (pool->next_task_index < pool->num_tasks) {
        task_data = pool->data[pool->next_task_index];
        pool->next_task_index++;
    } else {
        assert(pool->next_task_index == pool->num_tasks);
    }
    unlock_mutex(&pool->mut);

    return task_data;
}

THREAD_WORKER_RETURN_TYPE
simple_thread_pool_process(void * data) {
    simple_thread_data * tdata = (simple_thread_data *)data;
    simple_thread_pool * pool  = tdata->pool;

    lock_mutex(&pool->mut);
    arena ar = make_sub_arena(pool->ar, pool->mem_per_thr);
    unlock_mutex(&pool->mut);

    while (true) {
        void * task_data = simple_thread_pool_get_next_task(pool);
        if (task_data) {
            arena_reset(&ar);
            tdata->pool->worker_fn(task_data, &ar);
        } else {
            break;
        }
    }
    return 0;
}

void start_simple_thread_pool(simple_thread_pool * pool) {
    for (u64 i = 0; i < pool->num_thrs; i++) {
        pool->tdata[i].thr_index = i;
        pool->tdata[i].pool      = pool;
        pool->thrs[i] =
            make_thread(simple_thread_pool_process, pool->tdata + i);
    }

    for (u64 i = 0; i < pool->num_thrs; i++) {
        wait_for_thread(pool->thrs[i]);
    }
}

void do_threaded_tasks(simple_thread_pool_fn * worker_fn,
                       void **                 data,
                       u64                     num_tasks,
                       u64                     num_thrs,
                       u64                     mem_per_thr,
                       arena *                 ar) {
    assert(worker_fn);

    u64 mark = 0;
    if (ar) {
        mark = arena_pos(ar);
    }

    simple_thread_pool pool = create_simple_thread_pool(worker_fn,
                                                        data,
                                                        num_tasks,
                                                        num_thrs,
                                                        mem_per_thr,
                                                        ar);
    start_simple_thread_pool(&pool);
    destroy_simple_thread_pool(&pool);

    if (ar) {
        arena_pop_to_pos(ar, mark);
    }
}

////////////////////////////////////////////////////////////
// Tests

#ifdef TESTING

#define ARRAY_SIZE  100
#define NUM_THREADS 4

// The original function f(int n)
int f(int n) {
    // Example heavy computation: return the square root of n+1
    // You can replace this with your actual function logic.
    return n * n;
}

// Structure to pass multiple arguments to each thr function
typedef struct {
    int         start_index;
    int         end_index;
    const int * input_array;
    int *       output_array;
} thread_data;

// The thr function that processes a sub-range
THREAD_WORKER_RETURN_TYPE
process_range(void * arg) {
    thread_data * data = (thread_data *)arg;
    for (int i = data->start_index; i < data->end_index; ++i) {
        data->output_array[i] = f(data->input_array[i]);
    }
    return 0;
}

void test_thrs(void) {
    int         input_array[ARRAY_SIZE];
    int         output_array[ARRAY_SIZE];
    thread      thrs[NUM_THREADS];
    thread_data thr_data[NUM_THREADS];

    // Initialize the input array with some values
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        input_array[i] = i;
    }
    int chunk_size = ARRAY_SIZE / NUM_THREADS;

    // Create thrs and assign sub-ranges
    for (int i = 0; i < NUM_THREADS; ++i) {
        thr_data[i].start_index = i * chunk_size;
        // Ensure the last thr handles any remaining elements
        if (i == NUM_THREADS - 1) {
            thr_data[i].end_index = ARRAY_SIZE;
        } else {
            thr_data[i].end_index = (i + 1) * chunk_size;
        }
        thr_data[i].input_array  = input_array;
        thr_data[i].output_array = output_array;

        // Create the thr, passing a pointer to its data
        thrs[i] = make_thread(process_range, (void *)&thr_data[i]);
    }

    // Wait for all thrs to finish
    for (int i = 0; i < NUM_THREADS; ++i) {
        wait_for_thread(thrs[i]);
    }

    for (int i = 0; i < ARRAY_SIZE; ++i) {
        assert(output_array[i] == i * i);
    }
}

typedef struct {
    int   input;
    int * output;
} p_data;

void square_fn(void * data, arena * ar) {
    p_data * pdata = (p_data *)data;
    u64      mark  = arena_pos(ar);
    int *    ints  = arena_alloc_type_n(ar, int, 5);
    for (int i = 0; i < 5; i++) {
        ints[i] = pdata->input;
    }
    int result = 0;
    for (int i = 0; i < 5; i++) {
        result += ints[i];
    }
    *pdata->output = result;
    arena_pop_to_pos(ar, mark);
}

void test_simple_thr_pool(void) {
    int    input_array[ARRAY_SIZE];
    int    output_array[ARRAY_SIZE];
    thread thrs[NUM_THREADS];
    p_data data[ARRAY_SIZE];
    void * data_ptrs[ARRAY_SIZE];

    arena ar = make_arena(Mb(5));

    // Initialize the input array with some values
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        input_array[i] = i;
        data[i].input  = input_array[i];
        data[i].output = output_array + i;
        data_ptrs[i]   = data + i;
    }

    do_threaded_tasks(square_fn, data_ptrs, ARRAY_SIZE, 4, Mb(1), &ar);

    for (int i = 0; i < ARRAY_SIZE; ++i) {
        assert(output_array[i] == 5 * i);
        // printf("%d: %d\n", i, output_array[i]);
    }
}

#endif  // TESTING
