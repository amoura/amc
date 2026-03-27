#ifndef AM_OS_H
#define AM_OS_H

#include "common.h"
#include "mem.h"

#if defined(OS_LINUX) || defined(OS_MAC)
# include "os-linux.h"
#elif defined(OS_WINDOWS)
# include "os-windows.h"
#elif
# error Unknown OS
#endif

#define MAX_NUM_THREADS	16

typedef struct {
    char	*contents;
    size_t	len;
} Buffer;

Buffer
ReadWholeFile(const char* filename, Arena *arena);

Thread	MakeThread(
            ThreadWorker	*thread_worker,
            void		    *thread_data
        );
void	CloseThread(Thread		thread);
void	WaitForThread(Thread	thread);
Mutex	MakeMutex(void);
void	DestroyMutex(Mutex		*mutex);
void	LockMutex(Mutex			*mutex);
void	UnlockMutex(Mutex		*mutex);

struct SimpleThreadPool;
typedef struct SimpleThreadPool SimpleThreadPool;

typedef struct {
    u32	    		thread_index;
    SimpleThreadPool 	*pool;
} SimpleThreadData;

typedef void SimpleThreadPoolFn(void*, Arena*);

struct SimpleThreadPool {
    SimpleThreadPoolFn	*worker_fn;
    void		**data;
    u64			num_tasks;
    Thread		threads[MAX_NUM_THREADS];
    u64			num_threads;
    Mutex		mutex;
    u64			next_task_index;
    SimpleThreadData	tdata[MAX_NUM_THREADS];
    Arena		*arena;
    u64			mem_per_thread;
};

#endif // AM_OS_H
