#ifndef UTIL_H
#define UTIL_H

#include <limits.h>
#include <stdint.h>
#include <stdio.h>

/*
+---------------------------------------------------------+
|  INFO(ELI):                                             |
|                                                         |
|   Here are some types. I like using explicitly sized    |
|   types usually so here they are. If there are any      |
|   other basic types which would be cool we can add them |
|   later.                                                |
|                                                         |
+---------------------------------------------------------+ */

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;


/* 
+---------------------------------------------------------+
|  INFO:                                                  |
|                                                         |
|   Here are some nice helper functions.                  |
|   My goal is to make sized string functions just as     |
|   easy as null strings.                                 |
|                                                         |
|   There are two variants, a normal SString with         |
|   a maximum size of 4GB and a large size with           |
|   a maximum of Too big.                                 |
|                                                         |
|                                                         |
+---------------------------------------------------------+ */

typedef struct SString {
	i8* data;
	u32 size;
} SString;

typedef struct LString {
	i8* data;
	u64 size;
} LString;

#define isdigit(x) (x >= '0' && x <= '9')
#define isspace(x)                                                             \
	(x == ' ' || x == '\t' || x == '\n' || x == '\r' || x == '\v')

// macro to convert string literals to sstrings
#define sstring(x)                                                             \
	(SString) { (i8*)x, sizeof(x) - 1 }
#define lstring(x)                                                             \
	(LString) { (i8*)x, sizeof(x) - 1 }

u32 stou(const char* s);
u32 sstou(SString s);
u32 lstou(LString s);

SString SStrDup(SString s);
i32 SStrCmp(SString a, SString b);

/*
+----------------------------------------------------+
|     INFO:                                          |
|                                                    |
|    This is a custom print function. I thought it   |
|    would be fun, and it can be extended to handle  |
|    all kinds of data structures. I personally want |
|    to make it handle sized strings at least for    |
|    now.                                            |
+----------------------------------------------------+
*/

// raw fprintf replacement
void print(FILE* fd, const char* fmt, ...);

extern FILE* errfile;
extern FILE* logfile;

//helper specific print functions (implicit destination)
void errprint(const char* fmt, ...);
void logprint(const char* fmt, ...);

void setlogfile(FILE* file);
void seterrfile(FILE* file);


/*
Print formatting options
%d  -> 32 bit int
%l  -> 64 bit int
%ld -> 64 bit int
%f  -> 32 or 64 bit float
%p  -> pointer
%x  -> lower case hex
%X  -> upper case hex
%lx -> 64 bit hex
%lX -> 64 bit upper case hex
%%  -> single %


--- These ones are different than printf -----
%s -> SString
%n -> Null terminated string
*/

//nice logger with automatic newline
#define log(x, ...)  logprint("[LOG] %n:%d]:\t" x "\n", __FILE__, __LINE__,##__VA_ARGS__)
#define warn(x, ...) errprint("\033[38;2;255;255;0m[WARN] %n:%d]:\t" x "\033[0m\n", __FILE__, __LINE__,##__VA_ARGS__)
#define err(x, ...)  errprint("\033[38;2;255;0;0m[ERROR] %n:%d]:\t" x "\033[0m\n", __FILE__, __LINE__,##__VA_ARGS__)

//Fun fact, the popular spdlog library is actually the same speed as 
//printf so it provides no perf benefit

/*
+----------------------------------------------------+
|     INFO:                                          |
|                                                    |
|    Error checking here. Also functions for marking |
|    code as unreachable.                            |
+----------------------------------------------------+
*/

#include <assert.h>
#define panic() assert(0)
#define todo()                                                                 \
	{                                                                          \
		err("Not Implemented");                                                \
		panic();                                                               \
	}

#define REQ_ZERO(x) assert(!(x))

/*
+----------------------------------------------------+
|   INFO:                                            |
|                                                    |
|       Here is the custom memory allocator API.     |
|       I don't think we'll need it but it will      |
|       make it easy to do drop in replacements and  |
|       just overall reduce the number of heap       |
|       allocations required.                        |
+----------------------------------------------------+
*/

#define alloc_func_def(x)                                                      \
	void*(x)(u64 oldsize, u64 newsize, void* ptr, void* ctx)

typedef alloc_func_def(*alloc_func);

typedef struct Allocator {
	alloc_func a;
	void* ctx;
} Allocator;

#define Alloc(m, size) (log("Mem Alloc: %d", size), m.a(0, size, NULL, m.ctx))

#define Free(m, ptr, size) (log("Mem Free: %p", ptr), m.a(size, 0, ptr, m.ctx))

#define Realloc(m, ptr, oldsize, newsize)                                      \
	(log("Mem Realloc: %p %d", ptr, newsize), m.a(oldsize, newsize, ptr, m.ctx))

// Memory allocators
Allocator GlobalAllocatorCreate();

Allocator StackAllocatorCreate(const Allocator a, u64 minsize);
void StackAllocatorReset(Allocator* a);
void StackAllocatorDestroy(const Allocator* a);

/*
+------------------------------------------------------+
|   INFO:                                              |
|                                                      |
|   Some file handling functions.                      |
|   Make sure to discuss later about                   |
|   whether to use POSIX/stdlib or to use native       |
|   OS API                                             |
+------------------------------------------------------+
*/

// Dumps entire file into buffer
SString DumpFile(Allocator a, const char* filename);
void DumpFileS(SString* dst, SString filename);

// Write full contents of buffer into file
void WriteFile(const char* data);
void WriteFileS(SString data);

/*
+------------------------------------------------------+
|   INFO:                                              |
|                                                      |
|   Math functions. Maybe if we have a ton of these    |
|   we will move them to another file but should be    |
|   okay for now.                                      |
+------------------------------------------------------+
*/

typedef struct v2f {
	f32 x;
	f32 y;
} v2f;

typedef struct v2i {
	i32 x;
	i32 y;
} v2i;

typedef struct v2u {
	u32 x;
	u32 y;
} v2u;

#define CMPV2(a, b) (a.x == b.x && a.y == b.y)

u64 hash(u8* buf, u64 size);

#define MAX(a, b) \
    (a > b ? a : b)

#define MIN(a, b) \
    (a < b ? a : b)

#define CLAMP(t, min, max) \
    MAX(MIN(t, max), min) 

#define KB(x) (x * 1024)
#define MB(x) (x * 1024 * 1024)
#define GB(x) (x * 1024 * 1024 * 1024)

#endif
