In the utils header there are redefinitions
for explicitly sized integer and float types.

They follow the scheme of \[u|i\]\[size\].
So an unsigned byte woudl be u8, while a signed
64 bit number would be i64.

Floats come in f32 and f64 denoting whether they are
single(32) or double(64) bit precision.


## Strings
There is a sized string type which should be 
preffered internally.

It comes in two types, SString which has a 32 bit
size and LString which has a 64 bit size.

We shouldn't ever require LString but it exists for
completeness.

Alongsize SString there are SString versions of
strtou which is string to unsigned. I also have
StrDup and StrCmp which duplicate and compare
sized strings. These aren't yet complete but
they should be done in the next day or so.

## Memory

Memory allocation is handled via the Allocator
abstraction. An allocator is an allocation function
and a context pointer. To allocate use the macros
Alloc, Free, and Realloc which handle the
arguements.

To assist, there is a Stack allocator already in
the utilities which allows for arena allocation
style code.

There is also a Global Allocator which just forwards
the calls to the global allocator.

## Logging

There are three main logging macros, log, warn, and err.
Log is the basic print statement, it will log where
it was called from and will automatically append
a newline.

The other two macros will color the log output
yellow and red respectively.

All of these macros call through to the custom print
implementation which changes the behavior of printf
such that %n refers to null terminated strings, and 
%s refers to SStrings.

## Assertions

There are some assertion tools provided. Panic
will always crash the program and is just assert(0).

Todo will output a "Not Implemented" error before
panicking.

Require Zero will run the provided code and will
panic if a nonzero return value is provided. This
is useful for functions like strcmp or stat
which return nonzero on error.

## File I/O

Currently there are two main Functions planned for file I/O.
The first DumpFile reads an entire file and returns an SString.

The second is WriteFile and it takes a buffer and outputs the entire
thing to a file. Neither are implemented yet, and should
be working soon.

