#ifndef UTIL_H
#define UTIL_H

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

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float  f32;
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

//macro to convert string literals to sstrings
#define sstring(x) (SString){(i8*)x, sizeof(x)}
#define lstring(x) (LString){(i8*)x, sizeof(x)}

u32 stou(const char* s);
u32 sstou(SString s);
u32 lstou(LString s);

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

//raw fprintf replacement
void print(FILE* fd, const char* fmt, ...); 

/*
Print sub options
%d -> 32 bit int
%f -> 32 or 64 bit float
%p -> pointer
%x -> lower case hex
%X -> upper case hex
%% -> single %


--- These ones are different than printf -----
%s -> SString
%n -> Null terminated string

 
*/


//nice logger with automatic newline
#define log(x, ...)  print(stdout, "[LOG] %n:%d]:\t" x "\n", __FILE__, __LINE__,##__VA_ARGS__)
#define warn(x, ...) print(stderr, "\033[38;2;255;255;0m[WARN] %n:%d]:\t" x "\033[0m\n", __FILE__, __LINE__,##__VA_ARGS__)
#define err(x, ...)  print(stderr, "\033[38;2;255;0;0m[ERROR] %n:%d]:\t" x "\033[0m\n", __FILE__, __LINE__,##__VA_ARGS__)

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

#define alloc_func_def(x) \
    void* (x)(u64 oldsize, u64 newsize, void* ptr, void* ctx)

typedef alloc_func_def(*alloc_func);

typedef struct Allocator {
    alloc_func a;
    void* ctx;
} Allocator;

#define Alloc(m, size) \
    m.a(0, size, NULL, a.ctx)

#define Free(m, ptr, size) \
    m.a(size, 0, ptr, m.ctx)
    
#define Realloc(m, ptr, oldsize, newsize) \
    m.a(oldsize, newsize, ptr, a.ctx)


//Memory allocators
Allocator GlobalAllocatorCreate();

Allocator StackAllocatorCreate(const Allocator a, u64 minsize);
void StackAllocatorReset(Allocator* a);
void StackAllocatorDestroy(const Allocator* a);

#endif
