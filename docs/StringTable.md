Documentation for String Interning.

```c
typedef struct StringTable {
    Allocator mem; //should almost certainly be Global allocator

    u32* meta; //metadata used for Robin Hood Hashing
    u32* vals;
    u32 size;
    u32 cap;

    SString* strings;
    u32* entry;
    u32* freelist;

    u32 fsize;
    u32 ssize;

    u32 scap;


} StringTable;
```

This is the string table structure. When creating a new
table, only set the allocator and all other members should
be zero initialized.


```c
u32 StringAdd(StringTable* table, i8* string);
u32 StringAddS(StringTable* table, SString string);
```

These two functions are used to add strings. They
return an id which can be used to look up the string later
and is unique. If the string is already in the table it will
still return a valid id.

```c
SString StringDel(StringTable* table, u32 index);
```

This function takes in an index and removes it from the table.
It returns an SString which is used in case the string needs
to be freed or deallocated. The table does not assume the
string needs to be deallocated so it **must be freed manually**.

```c
void StringFree(StringTable* table);
```
This function frees all the memory required by the table,
make sure to delete any Strings which were allocated
before calling this.

```c
const SString StringGet(StringTable* table, u32 index);
```

This function can be used to grab the string from the table
given its unique index. The returned SString is const because
it's index depends on the contents. Basically to edit a string
you have to delete the current string and then re insert it. This
doesn't require freeing any memory, since StringDel doesn't
delete any memory.

```c
#define StringCmp(a, b)
```
This is a macro for comparing strings by their ids. It is equivalent to
`a == b` so yeah.
