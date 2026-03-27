#ifndef AM_OS_WINDOWS_H
#define AM_OS_WINDOWS_H

#include "common.h"
#include <windows.h>

typedef HANDLE Thread;
typedef HANDLE Mutex;
typedef DWORD THREAD_WORKER_RETURN_TYPE;
typedef THREAD_WORKER_RETURN_TYPE ThreadWorker(void *);

#endif // AM_OS_WINDOWS_H
