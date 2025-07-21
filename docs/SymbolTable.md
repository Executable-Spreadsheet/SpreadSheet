
```c
typedef struct SymbolEntry {
    SymbolType t; 
    u32 idx;
} SymbolEntry;
```
A symbol table contains symbol entries. Each symbol entry
contains a type and an index. The index is arbitrary and is
expected to point to an external buffer.

```c
typedef struct SymbolMap {
    Allocator mem;

    u32* keys;
    SymbolEntry* entries;
    u32 size;
    u32 cap;
} SymbolMap;
```
This is the data structure for storing the entries of a single
scope. It is a hashmap.



```c
typedef struct SymbolTable {
    Allocator mem;
    SymbolMap* scopes;
    u32 size;
    u32 cap;
} SymbolTable;
```

This is the global Symbol table. It is a stack of scopes which
store symbol entries.

## Symbol Types

```c
typedef enum SymbolType : u32 {
    S_INVALID = 0,
    S_VAR,
} SymbolType;
```

Currently there are two symbol types. By default Uninitialized
Symbol Entries are Invalid and must be initialized to some
value.

# Main Interface


```c
void SymbolInsert(SymbolTable* table, StrID key, SymbolEntry e);
```
This is the function for Writing to a SymbolEntry. I decided
to pass the SymbolEntries by value since they are only 8 bytes
in size which means they are the same size as a pointer. Passing
by value is good since pointers are unstable due to rehashing.

```c
SymbolEntry SymbolGet(SymbolTable* table, StrID key);
```

This is the function for reading a SymbolEntry.

```c
void SymbolPushScope(SymbolTable* table);
```

This will create a new SymbolMap and push it to the
top of the scope. Insertions always go to the top
of the scope, stack, and lookups start at the top
of the stack and go down if the symbol isn't found.

By default it will share its allocator with each new Scope.
This may be something we wish to change in the future.

```c
void SymbolPopScope(SymbolTable* table);
```

This will delete a scope. It frees the memory associated with
the scope and pops it off the stack.


## Exposed Internal functions

```c
u32 SymbolMapInsert(SymbolMap* map, StrID key);
```
This inserts a key into the SymbolMap, and returns an index
which can be used to reference the symbol entry.

```c
u32 SymbolMapGet(SymbolMap* map, StrID key);
```
This will lookup a key and return an index which can be
used to reference the symbol entry.


```c
void SymbolMapFree(SymbolMap* map);
```

This will free all the memory used by a symbol map.
