#include <windows.h>
#include <stdio.h>

thread make_thread(thread_worker *thr_worker,
            void *thr_data)
{
    return CreateThread(
        NULL,               // Default security
        0,                  // Default stack size
        thr_worker,       // Pointer to thr function
        thr_data,         // Argument to thr function
        0,                  // Default creation flags
        NULL                // Don't need the thr ID
    );
}

void close_thread(thread thr)
{
    BOOL ok = CloseHandle(thr);
    assert(ok);
}

void wait_for_thread(thread thr)
{
    WaitForSingleObject(thr, INFINITE);
}

mutex make_mutex(void)
{
    return CreateMutexA(NULL, FALSE, NULL);
}

void destroy_mutex(mutex *mut)
{
    BOOL ok = CloseHandle(*mut);
    assert(ok);
}

void lock_mutex(mutex *mut)
{
    WaitForSingleObject(*mut, INFINITE);
}

void unlock_mutex(mutex *mut)
{
    ReleaseMutex(*mut);
}

