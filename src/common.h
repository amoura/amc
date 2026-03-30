#ifndef AM_COMMON_H
#define AM_COMMON_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

////////////////////////////////////////////////////////////
// Aliases for basic types

// #ifndef bool
// typedef int bool;
// #endif
// #ifndef true
// #define true 1
// #endif
// #ifndef false
// #define false 0
// #endif

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float r32;
typedef double r64;


////////////////////////////////////////////////////////////
// Compiler and OS detection

// Compiler and architecture detection
#if   defined(__GNUC__)
# define COMPILER_GCC 1
#elif defined(__clang__)
# define COMPILER_CLANG 1
#elif defined(_MSC_VER)
# define COMPILER_MSVC 1
#else
# define COMPILER_UNKNOWN 1
#endif

// OS detection
#if   defined(__linux__)
#define OS_LINUX 1
#elif defined(__APPLE__) && defined(__MACH__)
#define OS_MAC 1
#elif defined(_WIN32)
#define OS_WINDOWS 1
#else
#define OS_UNKNOWN 1
#endif

#define WORD_SIZE (sizeof(int*))


////////////////////////////////////////////////////////////
// Helpful macros

#define stringify_(x)		#x
#define stringify(x)		stringify_(x)
#define join_(x,y)		x##y
#define join(x,y)		join_(x,y)

#define array_len(x)		(sizeof(x)/sizeof(*(x)))

#define int_from_ptr(ptr)		(size_t)((char*)ptr - (char*)0)
#define ptr_from_int(n)		(void*)((char*)0 + (n))

#define member(T,m)		(((T*)0)->m)
#define offset_of_member(T,m)	int_from_ptr(&member(T,m))

#define minimum(x,y)		(((x)<(y)) ? (x) : (y))
#define maximum(x,y)		(((x)>(y)) ? (x) : (y))
#define clamp(a,x,b)		(((x)<(a)) ? (a) : ((x)>(b)) ? (b) : (x))

#define Kb(x)			(1024*(x))
#define Mb(x)			(1024*1024*(x))
#define Gb(x)			(1024*1024*1024*(x))

#define for_i(T,i,n)    for(T i=0; i<(n); i++)
#define for_array(i,arr) for(u64 i=0; i<array_len(arr); i++)


////////////////////////////////////////////////////////////
// Alignment

#if !defined(ALIGN8)
# if defined(COMPILER_GCC)
#  define ALIGN8 __attribute__ ( (aligned (8)))
# else
#  define ALIGN8 __declspec (align (8))
# endif
#endif

#if !defined(ALIGN16)
# if defined(COMPILER_GCC)
#  define ALIGN16 __attribute__ ( (aligned (16)))
# else
#  define ALIGN16 __declspec (align (16))
# endif
#endif

#define align_up_pow2(x,p)	(((x) + (p) - 1) & ~((p) - 1))
#define align_down_pow2(x,p)	((x)&~((p) - 1))
#define is_aligned_pow2(ptr,p)	((int_from_ptr(ptr) & ((p) - 1)) == 0)


///////////////////////////////////////////////////////////
// Error reporting

#define error(msg)     do {\
    fprintf(stderr, "%s:%d: ", __FILE__, __LINE__);\
    fprintf(stderr, msg);\
    exit(1);\
} while(false)

#define errorf(fmt,...)     do {\
    fprintf(stderr, "%s:%d: ", __FILE__, __LINE__);\
    fprintf(stderr, fmt, __VA_ARGS__);\
    exit(1);\
} while(false)

#define error_if(cond,msg)  if (cond) {\
    error(msg);\
}

#define errorf_if(cond,fmt,...) if (cond) {\
    errorf(fmt, __VA_ARGS__);\
}

#endif // AM_COMMON_H
