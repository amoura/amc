#include "os-linux.h"

Thread
MakeThread(ThreadWorker	*thread_worker,
	     void		*thread_data)
{
    assert(thread_worker);
    Thread thread;
    int ret = pthread_create(&thread,
			     NULL,
			     thread_worker,
			     thread_data);
    assert(! ret);
    return thread;
}

void
CloseThread(Thread thread)
{
}

void
WaitForThread(Thread thread)
{
    pthread_join(thread, NULL);
}

Mutex
MakeMutex(void)
{
    Mutex mutex;
    int res = pthread_mutex_init(&mutex, NULL);
    assert(! res);
    return mutex;
}

void
DestroyMutex(Mutex	*mutex)
{
    int res = pthread_mutex_destroy(mutex);
    assert(! res);
}

void
LockMutex(Mutex		*mutex)
{
    int res = pthread_mutex_lock(mutex);
    assert(! res);
}

void
UnlockMutex(Mutex	*mutex)
{
    int res = pthread_mutex_unlock(mutex);
    assert(! res);
}

