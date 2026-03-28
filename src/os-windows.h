#ifndef AM_OS_WINDOWS_H
#define AM_OS_WINDOWS_H

#include "common.h"
#include <windows.h>

typedef HANDLE thread;
typedef HANDLE mutex;
typedef DWORD THREAD_WORKER_RETURN_TYPE;
typedef THREAD_WORKER_RETURN_TYPE thread_worker(void *);

#endif // AM_OS_WINDOWS_H
