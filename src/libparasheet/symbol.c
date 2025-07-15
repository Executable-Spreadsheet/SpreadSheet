#include <stdint.h>
#include <util/util.h>
#include <libparasheet/lib_internal.h>
#include <string.h>

static u32 MapInsertInternal(SymbolMap* map, StrID key, SymbolEntry e);

static void SymbolMapResize(SymbolMap* map) {
    u32 oldsize = map->cap; 

    while (map->size + 1 >= map->cap * MAX_LOAD_FACTOR) {
        map->cap = map->cap ? map->cap * 2 : 2; 
    }

    u32* okeys = map->keys; 
    SymbolEntry* oentries = map->entries;

    map->keys = Alloc(map->mem, map->cap * sizeof(u32));
    map->entries = Alloc(map->mem, map->cap * sizeof(SymbolEntry));

    memset(map->keys, -1, map->cap * sizeof(u32));
    memset(map->entries, 0, map->cap * sizeof(SymbolEntry)); //Technically not necessary but whatever

    map->size = 0;

    for (u32 i = 0; i < oldsize; i++) {
        if (okeys[i] != UINT32_MAX) {
            MapInsertInternal(map, okeys[i], oentries[i]);
        }
    }


    Free(map->mem, okeys, oldsize * sizeof(u32));
    Free(map->mem, oentries, oldsize * sizeof(SymbolEntry));
}

static u32 MapInsertInternal(SymbolMap* map, StrID key, SymbolEntry e) {
    if (map->size + 1 >= map->cap * MAX_LOAD_FACTOR)
        SymbolMapResize(map);

    u32 idx = hash((u8*)&key, sizeof(u32)) % map->cap;

    for (u32 i = 0; i < map->cap; i++) {
        u32 curr = map->keys[idx]; 

        if (StringCmp(key, curr)) {
            return idx;
        }

        if (curr == UINT32_MAX) {
            map->size++;
            map->keys[idx] = key;
            map->entries[idx] = e;
            return idx;
        }

        idx = (idx + 1) % map->cap;
    }


    panic();
}

u32 SymbolMapInsert(SymbolMap* map, StrID key) {
    return MapInsertInternal(map, key, (SymbolEntry){});
}

u32 SymbolMapGet(SymbolMap* map, StrID key) {
    if (!map->cap) return -1;

    u32 idx = hash((u8*)&key, sizeof(u32)) % map->cap;

    for (u32 i = 0; i < map->cap; i++) {
        u32 curr = map->keys[idx]; 

        if (StringCmp(key, curr)) {
            return idx;
        }

        if (curr == UINT32_MAX) {
            return -1;
        }

        idx = (idx + 1) % map->cap;
    }
    return -1;
}

void SymbolMapFree(SymbolMap* map) {
    Free(map->mem, map->keys, map->cap * sizeof(u32));
    Free(map->mem, map->entries, map->cap * sizeof(SymbolEntry));
}

void SymbolInsert(SymbolTable* table, StrID key, SymbolEntry e) {
    u32 idx = SymbolMapInsert(&table->scopes[table->size - 1], key);
    table->scopes[table->size - 1].entries[idx] = e;
}

SymbolEntry SymbolGet(SymbolTable* table, StrID key) {
    for (i32 i = table->size - 1; i >= 0; i--) {
        u32 e = SymbolMapGet(&table->scopes[i], key);
        if (e != UINT32_MAX) return table->scopes[i].entries[e];
    }

    return (SymbolEntry){0};
}

void SymbolPushScope(SymbolTable* table) {
    if (table->size + 1 > table->cap) {
        u32 oldsize = table->cap;
        table->cap = table->cap ? table->cap * 2 : 2;

        table->scopes = Realloc(table->mem, table->scopes,
                                oldsize * sizeof(SymbolMap),
                                table->cap * sizeof(SymbolMap));
    }

    table->scopes[table->size++] = (SymbolMap) {
        .mem = table->mem,
    };
}

void SymbolPopScope(SymbolTable* table) {
    SymbolMapFree(&table->scopes[--table->size]);
}
