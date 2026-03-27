#include <windows.h>
#include <stdio.h>

Thread MakeThread(ThreadWorker *thread_worker,
            void *thread_data)
{
    return CreateThread(
        NULL,               // Default security
        0,                  // Default stack size
        thread_worker,       // Pointer to thread function
        thread_data,         // Argument to thread function
        0,                  // Default creation flags
        NULL                // Don't need the thread ID
    );
}

void	CloseThread(Thread thread)
{
    BOOL ok = CloseHandle(thread);
    assert(ok);
}

void WaitForThread(Thread thread)
{
    WaitForSingleObject(thread, INFINITE);
}

Mutex MakeMutex(void)
{
    return CreateMutexA(NULL, FALSE, NULL);
}

void DestroyMutex(Mutex *mutex)
{
    BOOL ok = CloseHandle(*mutex);
    assert(ok);
}

void LockMutex(Mutex *mutex)
{
    WaitForSingleObject(*mutex, INFINITE);
}

void UnlockMutex(Mutex *mutex)
{
    ReleaseMutex(*mutex);
}


#if 0
#define NUM_THREADS 4
#define ARRAY_SIZE 1000

// Struct to pass data to each thread
typedef struct {
    int start;
    int end;
    int* input;
    double* output;
} ThreadData;

// The function to run in parallel
double f(int n) {
    return (double)n * 3.14159; 
}

// Thread procedure following the Windows API signature
DWORD WINAPI WorkerThread(LPVOID lpParam) {
    ThreadData* data = (ThreadData*)lpParam;
    for (int i = data->start; i < data->end; i++) {
        data->output[i] = f(data->input[i]);
    }
    return 0;
}

int main() {
    int input[ARRAY_SIZE];
    double output[ARRAY_SIZE];
    HANDLE threads[NUM_THREADS];
    ThreadData t_data[NUM_THREADS];

    // Initialize input array
    for (int i = 0; i < ARRAY_SIZE; i++) input[i] = i;

    int chunkSize = ARRAY_SIZE / NUM_THREADS;

    for (int i = 0; i < NUM_THREADS; i++) {
        t_data[i].start = i * chunkSize;
        // Ensure the last thread covers any remainder
        t_data[i].end = (i == NUM_THREADS - 1) ? ARRAY_SIZE : (i + 1) * chunkSize;
        t_data[i].input = input;
        t_data[i].output = output;

        // Create the thread using native Win32 API
        threads[i] = CreateThread(
            NULL,               // Default security
            0,                  // Default stack size
            WorkerThread,       // Pointer to thread function
            &t_data[i],         // Argument to thread function
            0,                  // Default creation flags
            NULL                // Don't need the thread ID
        );
    }

    // Wait for all threads to finish
    WaitForMultipleObjects(NUM_THREADS, threads, TRUE, INFINITE);

    // Clean up handles
    for (int i = 0; i < NUM_THREADS; i++) {
        CloseHandle(threads[i]);
    }

    printf("Done. Result[999] = %f\n", output[999]);
    return 0;
}
#endif
