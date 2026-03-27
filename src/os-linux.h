#ifndef AM_OS_LINUX_H
#define AM_OS_LINUX_H

#include "common.h"
#include <pthread.h>

////////////////////////////////////////////////////////////
// Threads

#include <pthread.h>

typedef	pthread_t                   Thread;
typedef pthread_mutex_t	            Mutex;
typedef void *				        THREAD_WORKER_RETURN_TYPE;
typedef THREAD_WORKER_RETURN_TYPE 	ThreadWorker(void *);

#endif // AM_OS_LINUX_H
