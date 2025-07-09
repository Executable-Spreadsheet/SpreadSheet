# Util.h information

Here is an overview of each function and data type
in the util.h header.


## Basic Data Types

Inspired by Rust and modern langauges explicit
size typedefs are provided.

Here are the typedefs

```c
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
```

Please use these types, they are platform independent and
explicitly sized.


## Sized Strings

There are two provided Sized String datatypes.

```c
typedef struct SString {
    i8* data;
    u32 size;
} SString;

typedef struct LString {
    i8* data;
    u64 size;
} LString;
```

SString has better support currently since it is highly
unlikely that we'll ever need a buffer larger than
~4 GB.

Alongside the sized string type there are some basic functions.

```c
#define isdigit(x) (x >= '0' && x <= '9')

//macro to convert string literals to sstrings
#define sstring(x) (SString){(i8*)x, sizeof(x)}
#define lstring(x) (LString){(i8*)x, sizeof(x)}

u32 stou(const char* s);
u32 sstou(SString s);
u32 lstou(LString s);

SString SStrDup(SString s);
SString SStrCmp(SString a, SString b);
```

These for the most part have the same functionality as the
standard library functions they share a name with. The
ssstring and lstring macros are used for easily creating
sized string constants.

### print

For this project there is a custom print function.

It is essentially the same as printf but with the following
supported substitutions.

|code|type                      |
|----|--------------------------|
| %d | 32 bit int               |
| %l | 64 bit int               |
| %ld| 64 bit int               |
| %f | 32 or 64 bit int         |
| %p | pointer (64 bit)         |
| %x | lower case 32 bit hex    |
| %X | upper case 32 bit hex    |
| %lx| lower case 64 bit hex    |
| %lX| upper case 64 bit hex    |
| %% | %                        |
| %s | SString                  |
| %s | Null terminated string   |


To make print even more powerful three macros are provided
for different types of logging.

```c
#define log(x, ...)
#define warn(x, ...)
#define err(x, ...)
```

They respectively produce the following output lines:

```
[LOG] src/util/util.c:100]: A logging message
[WARN] src/util/util.c:100]: A logging message
[ERROR] src/util/util.c:100]: A logging message
```

In addition both warn and err are color coded 
yellow and red respectively.


## Error Checking

Util.h includes assert.h from the standard library.
It also provides several helper macros.

```c
#define panic()
```

Which marks code as unreachable and will forcibly crash the
program when reached.

```c
#define todo()
```

This is like panic but also will output Not Implemented to
the log.

```c
#define REQ_ZERO(x)
```

This is a helper macro for functions which return success
on zero and should crash the program if they fail.
It expands to `assert(!x)`.

## Memory Allocator API

util.h has a custom memory allocator API which allows for easy
implementation and swapping of custom allocators.

For usage there are only three things which matter,
```c
#define Alloc(m, size)
#define Free(m, ptr, size)
#define Realloc(m, ptr, oldsize, newsize)
```

They work exactly as expected. The first parameter in each
is a memory allocator which has the type `Allocator`.


#### Writing Custom Allocators

To add a new allocator all you need to do is implement
the alloc_function which has the following signature.

```c
    void* my_alloc(u64 oldsize, u64 newsize, void* ptr, void* ctx)
```

There are some basic implementations in util.c, you can also
just ask me (Eli). 

Generally speaking make sure to add a create function if your
allocator has ownership over some context.

Current List of Allocators
- Stack Allocator
- Global Allocator (pass through to malloc, realloc, free)

## File Functions

There are currently two file functions

```c
//Dumps entire file into buffer
SString DumpFile(Allocator a, const char* filename);
void DumpFileS(SString* dst, SString filename);

//Write full contents of buffer into file
void WriteFile(const char* data);
void WriteFileS(SString data);
```

They respectively load an entire file into a buffer and
write out a buffer into a file.

## Math Functions

There are some convenience types included as well.

```c
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
```

In addition there is a macro for comparing the vec2 types.

```c
#define CMPV2(a, b)
```

In addition there is a hash function provided

```c
u64 hash(u8* buf, u64 size);
```

As of 7/9 it uses FNV-1a. 
