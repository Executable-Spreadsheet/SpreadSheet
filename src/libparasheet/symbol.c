#include <libparasheet/lib_internal.h>
#include <stdint.h>
#include <string.h>
#include <util/util.h>

// Forward declaration for Indirect Recursion
static u32 MapInsertInternal(SymbolMap* map, StrID key, SymbolEntry e);

// Resize the symbol hash table
static void SymbolMapResize(SymbolMap* map) {
	u32 oldsize = map->cap;

	while (map->size + 1 >= map->cap * MAX_LOAD_FACTOR) {
		map->cap = map->cap ? map->cap * 2 : 2;
	}

    StrID* okeys = map->keys; 
    SymbolEntry* oentries = map->entries;

    map->keys = Alloc(map->mem, map->cap * sizeof(StrID));
    map->entries = Alloc(map->mem, map->cap * sizeof(SymbolEntry));

    memset(map->keys, -1, map->cap * sizeof(StrID));
    memset(map->entries, 0, map->cap * sizeof(SymbolEntry)); //Technically not necessary but whatever

	map->size = 0;

	for (u32 i = 0; i < oldsize; i++) {
		if (okeys[i].idx != UINT32_MAX || okeys[i].gen != UINT32_MAX) {
			MapInsertInternal(map, okeys[i], oentries[i]);
		}
	}

	Free(map->mem, okeys, oldsize * sizeof(u32));
	Free(map->mem, oentries, oldsize * sizeof(SymbolEntry));
}

// Insertion which supports writing entries directly. Used in both
// the public SymbolInsert and the internal Resize
static u32 MapInsertInternal(SymbolMap* map, StrID key, SymbolEntry entry) {
	if (map->size + 1 >= map->cap * MAX_LOAD_FACTOR)
		SymbolMapResize(map);

	u32 idx = hash((u8*)&key, sizeof(u32)) % map->cap;

    for (u32 i = 0; i < map->cap; i++) {
        StrID curr = map->keys[idx]; 

		if (StringCmp(key, curr)) {
			return idx;
		}

		if (curr.idx == UINT32_MAX && curr.gen == UINT32_MAX) {
			map->size++;
			map->keys[idx] = key;
			map->entries[idx] = entry;
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
	if (!map->cap)
		return -1;

	u32 idx = hash((u8*)&key, sizeof(u32)) % map->cap;

    for (u32 i = 0; i < map->cap; i++) {
        StrID curr = map->keys[idx]; 

		if (StringCmp(key, curr)) {
			return idx;
		}

		if (curr.idx == UINT32_MAX && curr.gen == UINT32_MAX) {
			return -1;
		}

		idx = (idx + 1) % map->cap;
	}
	return -1;
}

void SymbolMapFree(SymbolMap* map) {
	Free(map->mem, map->keys, map->cap * sizeof(StrID));
	Free(map->mem, map->entries, map->cap * sizeof(SymbolEntry));
}

void SymbolInsert(SymbolTable* table, StrID key, SymbolEntry entry) {
	u32 idx = SymbolMapInsert(&table->scopes[table->size - 1], key);
	table->scopes[table->size - 1].entries[idx] = entry;
}

SymbolEntry SymbolGet(SymbolTable* table, StrID key) {
	for (i32 i = table->size - 1; i >= 0; i--) {
		u32 e = SymbolMapGet(&table->scopes[i], key);
		if (e != UINT32_MAX)
			return table->scopes[i].entries[e];
	}

	return (SymbolEntry){0};
}

void SymbolPushScope(SymbolTable* table) {
	if (table->size + 1 > table->cap) {
		u32 oldsize = table->cap;
		table->cap = table->cap ? table->cap * 2 : 2;

        SymbolMap* temp = Realloc(table->mem, table->scopes, oldsize * sizeof(SymbolMap),
                    table->cap * sizeof(SymbolMap));
        if (!temp) {
            Free(table->mem, table->scopes, oldsize * sizeof(SymbolMap)); 
            err("Failed to realloc!");
            panic();
        }


		table->scopes = temp;
	}

	table->scopes[table->size++] = (SymbolMap){
		.mem = table->mem,
	};
}

void SymbolPopScope(SymbolTable* table) {
	SymbolMapFree(&table->scopes[--table->size]);
}
