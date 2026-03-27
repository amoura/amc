#include <stdio.h>
#include "common.h"
#include "mem.h"
#include "os.h"

#if defined(OS_LINUX) || defined(OS_MAC)
# include "os-linux.c"
#elif defined(OS_WINDOWS)
# include "os-windows.c"
#elif
# error Unknown OS
#endif


////////////////////////////////////////////////////////////
// File I/O

Buffer
ReadWholeFile(const char* filename, Arena *arena)
{
    FILE* fp = NULL;
    long bufsize = 0;
    char* source = NULL;
    Buffer buffer = {0};

    fp = fopen(filename, "rb");
    assert(fp);

    int fseek_ret = fseek(fp, 0L, SEEK_END);
    assert(fseek_ret == 0);

    bufsize = ftell(fp);
    if (bufsize == -1) {
        perror("Error getting file size");
        fclose(fp);
        assert(false);
    }

    if (fseek(fp, 0L, SEEK_SET) != 0) {
        perror("Error seeking to start of file");
        fclose(fp);
        assert(false);
    }

    source = ArenaAllocTypeN(arena, char, bufsize + 1);
    u64 mark = ArenaPos(arena);
    size_t len = fread(source, sizeof(char), bufsize, fp);
    if (ferror(fp) != 0) {
        fputs("Error reading file", stderr);
        ArenaPopToPos(arena, mark);
        fclose(fp);
	assert(false);
    } else {
        source[len] = 0;
    }
    
    // Close the file
    fclose(fp);

    buffer.len = len;
    buffer.contents = source;
    return buffer;
}


////////////////////////////////////////////////////////////
// Multi-threading

SimpleThreadPool
CreateSimpleThreadPool(SimpleThreadPoolFn	*worker_fn,
		       void	    		**data,
		       u64	    		num_tasks,
		       u64	    		num_threads,
		       u64			mem_per_thread,
		       Arena			*arena)
{
    SimpleThreadPool pool = {0};
    assert(num_threads <= ArrayLen(pool.threads));
    
    pool.worker_fn = worker_fn;
    pool.data = data;
    pool.num_tasks = num_tasks;
    pool.num_threads = num_threads;
    pool.next_task_index = 0;
    pool.mutex = MakeMutex();
    pool.arena = arena;
    pool.mem_per_thread = mem_per_thread;
    
    return pool;
}

void
DestroySimpleThreadPool(SimpleThreadPool	*pool)
{
    for (u64 i=0; i<pool->num_threads; i++) {
        CloseThread(pool->threads[i]);
    }
    DestroyMutex(&pool->mutex);
}

void *
SimpleThreadPoolGetNextTask(SimpleThreadPool	*pool)
{
    LockMutex(&pool->mutex);
    assert(pool->next_task_index < pool->num_tasks + 1);
    void *task_data = NULL;
    if (pool->next_task_index < pool->num_tasks) {
	task_data = pool->data[pool->next_task_index];	
	pool->next_task_index++;
    } else {
	assert(pool->next_task_index == pool->num_tasks);
    }
    UnlockMutex(&pool->mutex);

    return task_data;
}

THREAD_WORKER_RETURN_TYPE
SimpleThreadPoolProcess(void *data)
{
    SimpleThreadData 	*tdata = (SimpleThreadData *) data;
    SimpleThreadPool 	*pool  = tdata->pool;

    LockMutex(&pool->mutex);
    Arena 		arena  = MakeSubArena(pool->arena,
					      pool->mem_per_thread);
    UnlockMutex(&pool->mutex);
    
    while (true) {
	void *task_data = SimpleThreadPoolGetNextTask(pool);
	if (task_data) {
	    ArenaReset(&arena);
	    tdata->pool->worker_fn(task_data, &arena);
	} else {
	    break;
	}
    }
    return 0;
}

void
StartSimpleThreadPool(SimpleThreadPool	*pool)
{
    for (u64 i=0; i<pool->num_threads; i++) {
	pool->tdata[i].thread_index = i;
	pool->tdata[i].pool = pool;
	pool->threads[i] = MakeThread(SimpleThreadPoolProcess,
					pool->tdata + i);
    }

    for (u64 i=0; i<pool->num_threads; i++) {
	WaitForThread(pool->threads[i]);
    }
}

void
DoThreadedTasks(SimpleThreadPoolFn	*worker_fn,    
		void	    		**data,	       
		u64	    		num_tasks,     
		u64	    		num_threads,   
		u64			mem_per_thread,
		Arena			*arena)
{
    assert(worker_fn);
    
    u64 mark = 0;
    if (arena) {
	mark = ArenaPos(arena);	
    }
    
    SimpleThreadPool pool = CreateSimpleThreadPool(worker_fn,
						   data,
						   num_tasks,
						   num_threads,
						   mem_per_thread,
						   arena);
    StartSimpleThreadPool(&pool);
    DestroySimpleThreadPool(&pool);

    if (arena) {
	ArenaPopToPos(arena, mark);	
    }
}

////////////////////////////////////////////////////////////
// Tests

#ifdef TESTING

#define ARRAY_SIZE 100
#define NUM_THREADS 4

// The original function f(int n)
int f(int n) {
    // Example heavy computation: return the square root of n+1
    // You can replace this with your actual function logic.
    return n * n; 
}

// Structure to pass multiple arguments to each thread function
typedef struct {
    int start_index;
    int end_index;
    const int *input_array;
    int *output_array;
} ThreadData;

// The thread function that processes a sub-range
THREAD_WORKER_RETURN_TYPE
process_range(void *arg) {
    ThreadData* data = (ThreadData *) arg;
    for (int i = data->start_index; i < data->end_index; ++i) {
        data->output_array[i] = f(data->input_array[i]);
    }
    return 0;
}

void test_threads(void) {
    int 	input_array[ARRAY_SIZE];
    int 	output_array[ARRAY_SIZE];
    Thread 	threads[NUM_THREADS];
    ThreadData 	thread_data[NUM_THREADS];

    // Initialize the input array with some values
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        input_array[i] = i; 
    }
    int chunk_size = ARRAY_SIZE / NUM_THREADS;

    // Create threads and assign sub-ranges
    for (int i = 0; i < NUM_THREADS; ++i) {
        thread_data[i].start_index = i * chunk_size;
        // Ensure the last thread handles any remaining elements
        if (i == NUM_THREADS - 1) {
            thread_data[i].end_index = ARRAY_SIZE;
        } else {
            thread_data[i].end_index = (i + 1) * chunk_size;
        }
        thread_data[i].input_array = input_array;
        thread_data[i].output_array = output_array;

        // Create the thread, passing a pointer to its data
        threads[i] = MakeThread(process_range,
				  (void*)&thread_data[i]);
    }

    // Wait for all threads to finish
    for (int i = 0; i < NUM_THREADS; ++i) {
        WaitForThread(threads[i]);
    }

    for (int i = 0; i < ARRAY_SIZE; ++i) {
	assert(output_array[i] == i*i);
    }
}


typedef struct {
    int	input;
    int	*output;
} PData;

void
square_fn(void *data, Arena *arena)
{
    PData *pdata = (PData *) data;
    u64 mark = ArenaPos(arena);
    int *ints = ArenaAllocTypeN(arena, int, 5);
    for (int i=0; i<5; i++) {
	ints[i] = pdata->input;
    }
    int result = 0;
    for (int i=0; i<5; i++) {
	result += ints[i];
    }
    *pdata->output = result;
    ArenaPopToPos(arena, mark);
}

void test_simple_thread_pool(void) {
    int 	input_array[ARRAY_SIZE];
    int 	output_array[ARRAY_SIZE];
    Thread 	threads[NUM_THREADS];
    PData 	data[ARRAY_SIZE];
    void	*data_ptrs[ARRAY_SIZE];

    Arena arena = MakeArena(Mb(5));

    // Initialize the input array with some values
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        input_array[i] = i;
	data[i].input = input_array[i];
	data[i].output = output_array + i;
	data_ptrs[i] = data + i;
    }

    DoThreadedTasks(square_fn, 
		    data_ptrs, 
		    ARRAY_SIZE,
		    4,	      
		    Mb(1),     
		    &arena);
    
    for (int i = 0; i < ARRAY_SIZE; ++i) {
	assert(output_array[i] == 5*i);
	// printf("%d: %d\n", i, output_array[i]);
    }
}

#endif // TESTING
