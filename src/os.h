#ifndef AM_OS_H
#define AM_OS_H

#include "common.h"
#include "mem.h"

#if defined(OS_LINUX) || defined(OS_MAC)
#include "os-linux.h"
#elif defined(OS_WINDOWS)
#include "os-windows.h"
#elif
#error Unknown OS
#endif

#define MAX_NUM_THREADS 16

typedef struct {
    char * contents;
    size_t len;
} buffer;

buffer read_whole_file(const char * filename, arena * ar);

typedef struct {
    char * name;
    bool   is_dir;
} file_entry;

thread make_thread(thread_worker * thr_worker, void * thr_data);
void   close_thread(thread thr);
void   wait_for_thread(thread thr);
mutex  make_mutex(void);
void   destroy_mutex(mutex * mut);
void   lock_mutex(mutex * mut);
void   unlock_mutex(mutex * mut);

struct simple_thread_pool;
typedef struct simple_thread_pool simple_thread_pool;

typedef struct {
    u32                  thr_index;
    simple_thread_pool * pool;
} simple_thread_data;

typedef void simple_thread_pool_fn(void *, arena *);

struct simple_thread_pool {
    simple_thread_pool_fn * worker_fn;
    void **                 data;
    u64                     num_tasks;
    thread                  thrs[MAX_NUM_THREADS];
    u64                     num_thrs;
    mutex                   mut;
    u64                     next_task_index;
    simple_thread_data      tdata[MAX_NUM_THREADS];
    arena *                 ar;
    u64                     mem_per_thr;
};

#endif  // AM_OS_H
