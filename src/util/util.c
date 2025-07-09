#include <util/util.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//----- String Conversions ---------

u32 stou(const char *s) {
  u32 out = 0;

  while (s[0] && isdigit(s[0])) {
    out *= 10;
    out += s[0] - '0';
    s++;
  }

  return out;
}

u32 sstou(SString s) {
  u32 out = 0;
  u32 i = 0;

  while (i < s.size && isdigit(s.data[i])) {
    out *= 10;
    out += s.data[i] - '0';
    i++;
  }

  return out;
}

u32 lstou(LString s) {
  u64 out = 0;
  u64 i = 0;

  while (i < s.size && isdigit(s.data[i])) {
    out *= 10;
    out += s.data[i] - '0';
    i++;
  }

  return out;
}

//------------- Custom Printf ------------------

static const char *print_arg(FILE *fd, u32 precision, const char *fmt,
                             va_list args) {

  switch (fmt[1]) {
  case '%': {
    putc('%', fd);
  } break;
  case 'n': {
    i8 *c = va_arg(args, i8 *);

    while (c[0]) {
      putc(c[0], fd);
      c++;
    }
  } break;
  case 's': {
    SString v = va_arg(args, SString);

    for (u32 i = 0; i < v.size; i++) {
      putc(v.data[i], fd);
    }

  } break;
  case 'p': {
    u64 p = va_arg(args, u64);

    if (!p) {
      putc('(', fd);
      putc('n', fd);
      putc('i', fd);
      putc('l', fd);
      putc(')', fd);
      break;
    }

    putc('0', fd);
    putc('x', fd);

    u32 num_digits = 0;
    u64 temp = p;
    while (temp) {
      num_digits += 1;
      temp >>= 4;
    }

    for (i32 i = num_digits - 1; i >= 0; i--) {
      u8 v = (p >> (4 * i)) & 0xF;
      if (v < 10)
        putc(v + '0', fd);
      else
        putc(v + ('a' - 10), fd);
    }
  } break;
  case 'd': {
    u32 num_digits = 1;
    i32 n = va_arg(args, i32);
    if (n < 0) {
      n *= -1;
      putc('-', fd);
    }
    u32 val = n;

    u32 temp = val / 10;
    while (temp) {
      temp /= 10;
      num_digits *= 10;
    }

    while (num_digits) {
      i8 digit = ((val % (num_digits * 10)) / num_digits) + '0';
      num_digits /= 10;
      putc(digit, fd);
    }
  } break;
  case 'l': {
    switch (fmt[2]) {
    case 'X': {
      fmt++;
      u64 p = va_arg(args, u64);

      u32 num_digits = 0;
      u64 temp = p;
      while (temp) {
        num_digits += 1;
        temp >>= 4;
      }

      for (i32 i = num_digits - 1; i >= 0; i--) {
        u8 v = (p >> (4 * i)) & 0xF;
        if (v < 10)
          putc(v + '0', fd);
        else
          putc(v + ('A' - 10), fd);
      }
    } break;
    case 'x': {
      fmt++;
      u64 p = va_arg(args, u64);

      u32 num_digits = 0;
      u64 temp = p;
      while (temp) {
        num_digits += 1;
        temp >>= 4;
      }

      for (i32 i = num_digits - 1; i >= 0; i--) {
        u8 v = (p >> (4 * i)) & 0xF;
        if (v < 10)
          putc(v + '0', fd);
        else
          putc(v + ('a' - 10), fd);
      }
    } break;
    case 'd':
      fmt++;
    default: {
      u32 num_digits = 1;
      i64 n = va_arg(args, i64);
      if (n < 0) {
        n *= -1;
        putc('-', fd);
      }
      u32 val = n;

      u32 temp = val / 10;
      while (temp) {
        temp /= 10;
        num_digits *= 10;
      }

      while (num_digits) {
        i8 digit = ((val % (num_digits * 10)) / num_digits) + '0';
        num_digits /= 10;
        putc(digit, fd);
      }
    } break;
    }
  }
  case 'x': {
    u32 p = va_arg(args, u32);

    u32 num_digits = 0;
    u64 temp = p;
    while (temp) {
      num_digits += 1;
      temp >>= 4;
    }

    for (i32 i = num_digits - 1; i >= 0; i--) {
      u8 v = (p >> (4 * i)) & 0xF;
      if (v < 10)
        putc(v + '0', fd);
      else
        putc(v + ('a' - 10), fd);
    }
  } break;
  case 'X': {
    u32 p = va_arg(args, u32);

    u32 num_digits = 0;
    u64 temp = p;
    while (temp) {
      num_digits += 1;
      temp >>= 4;
    }

    for (i32 i = num_digits - 1; i >= 0; i--) {
      u8 v = (p >> (4 * i)) & 0xF;
      if (v < 10)
        putc(v + '0', fd);
      else
        putc(v + ('A' - 10), fd);
    }
  } break;
  case 'f': {
    u64 num_digits = 1;
    f64 val = va_arg(args, f64);

    if (val < 0) {
      putc('-', fd);
      val *= -1;
    }

    // print integer part
    u32 integer = val;
    val -= integer;
    u32 temp = integer / 10;
    while (temp) {
      temp /= 10;
      num_digits *= 10;
    }

    while (num_digits) {
      i8 digit = ((integer % (num_digits * 10)) / num_digits) + '0';
      num_digits /= 10;
      putc(digit, fd);
    }

    // print fractional part
    if (precision)
      putc('.', fd);

    num_digits = 1;
    for (u32 i = 0; i < precision; i++) {
      val *= 10;
      num_digits *= 10;
    }
    num_digits /= 10;
    u32 frac = (u32)val;
    val -= frac;
    frac += val >= 0.5 ? 1 : 0;

    while (num_digits) {
      i8 digit = ((frac % (num_digits * 10)) / num_digits) + '0';
      num_digits /= 10;
      putc(digit, fd);
    }

  } break;
  default: {
    todo();
  } break;
  }
  fmt += 2;
  return fmt;
}

void print(FILE *fd, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  while (fmt[0]) {

    switch (fmt[0]) {
    case '%': {
      u32 precision = 4;
      if (fmt[1] == '.') {
        fmt += 1;
        precision = 0;

        while (fmt[1] && isdigit(fmt[1])) {
          precision *= 10;
          precision += fmt[1] - '0';
          fmt++;
        }
      }
      fmt = print_arg(fd, precision, fmt, args);
    } break;
    default: {
      putc(fmt[0], fd);
      fmt++;
    } break;
    }
  }

  va_end(args);
}

//------------ Memory Allocators --------------

// Global Allocator (malloc, calloc, etc.)
static alloc_func_def(GlobalAllocate) {
  if (oldsize == 0 && ptr == 0 && newsize == 0) {
        return 0;
  }

  if (oldsize == 0) {
    return malloc(newsize);
  }

  if (ptr && newsize == 0) {
    free(ptr);
    return 0;
  }

  if (ptr) {
    return realloc(ptr, newsize);
  }

  panic();
}

Allocator GlobalAllocatorCreate() {
  return (Allocator){
      .a = GlobalAllocate,
  };
}

// Stack/Bump Allocator

// inline because it requires less indirection
typedef struct StackAllocator {
  Allocator a; // for destruction
  u64 cap;
  u64 size;
  u64 data[];
} StackAllocator;

static alloc_func_def(StackAllocate) {
  StackAllocator *s = ctx;

  if (oldsize == 0) {
    log("Stack Alloc: %p %d", ctx, newsize);
    if (s->size + newsize > s->cap)
      return 0; // error
    void *out = &s->data[s->size];
    s->size += newsize;

    return out;
  }

  // realloc works via an alloc + memcpy
  if (ptr && oldsize && newsize) {
    log("Stack Realloc: %p (%p, %d) -> %d", ctx, ptr, oldsize, newsize);
    if (s->size + newsize > s->cap)
      return ptr; // error

    void *dst = &s->data[s->size];
    s->size += newsize;

    memcpy(dst, ptr, oldsize);
    return dst;
  }

  if (ptr) {
    log("Stack Free: %p (%p, %d)", ctx, ptr, oldsize);
    // free
    return 0;
  }

  panic();
}

// requires additional allocator.
// Not zero initialized
Allocator StackAllocatorCreate(const Allocator a, u64 minsize) {

  StackAllocator *s = Alloc(a, sizeof(StackAllocator) + minsize);
  s->a = a;
  s->size = 0;
  s->cap = minsize;

  return (Allocator){
      .a = StackAllocate,
      .ctx = s,
  };
}

void StackAllocatorReset(Allocator *a) {
  StackAllocator *s = a->ctx;
  s->size = 0;
}

void StackAllocatorDestroy(const Allocator *a) {
  StackAllocator *s = a->ctx;
  Free(s->a, s, sizeof(StackAllocator) + s->cap);
}

// Dumps entire file into buffer
#include "stdio.h"
#include "sys/stat.h"

SString DumpFile(Allocator a, const char *filename) {
  FILE *f = fopen(filename, "r");

  struct stat info;
  if (stat(filename, &info)) {
    err("Failed to stat file");
    panic();
  }

  SString output = {.size = info.st_size, .data = Alloc(a, info.st_size)};

  fread(output.data, 1, info.st_size, f);
  fclose(f);

  return output;
}

void DumpFileS(SString *dst, SString filename) { todo(); }

// Write full contents of buffer into file
void WriteFile(const char *data) { todo(); }

void WriteFileS(SString data) { todo(); }


//INFO(ELI): Going to go with FNV-1a since its fast
//and easy to implement.
u64 hash(u8* buf, u64 size) {
    u64 hash = 14695981039346656037UL;
    for (u64 i = 0; i < size; i++) {
        hash ^= buf[i];
        hash = hash * 1099511628211UL;
    }
    return hash;
}


