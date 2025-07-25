#include <libparasheet/lib_internal.h>
#include <util/util.h>

int main() {

	SymbolMap map = {
		.mem = GlobalAllocatorCreate(),
	};

	for (u32 i = 0; i < 10; i++) {
		StrID insert = {.idx = i, .gen = 1};
		u32 e = SymbolMapInsert(&map, insert);
		map.entries[e] = (SymbolEntry){
			.type = 0,
			.data.d.i = i,
		};
	}

	for (u32 i = 0; i < 10; i++) {
		StrID insert = {.idx = i, .gen = 1};
		u32 e = SymbolMapInsert(&map, insert);
		map.entries[e] = (SymbolEntry){
			.type = 0,
			.data.d.i = i * 2,
		};
	}

	print(stdout, "Table:\n");
	for (u32 i = 0; i < map.cap; i++) {
		print(stdout, "\t(%d %d)\n", map.keys[i], map.entries[i].data.d.i);
	}

	print(stdout, "Vals:\n");
	for (u32 i = 0; i < 10; i++) {
		StrID insert = {.idx = i, .gen = 1};
		u32 e = SymbolMapGet(&map, insert);
		print(stdout, "\t(%d %d)\n", i, map.entries[e].data.d.i);
	}

	SymbolMapFree(&map);

	SymbolTable t = {
		.mem = GlobalAllocatorCreate(),
	};
	SymbolPushScope(&t);

	for (u32 i = 0; i < 10; i++) {
		StrID insert = {.idx = i, .gen = 1};
		SymbolInsert(&t, insert, (SymbolEntry){.type = S_VAR, .data.d.i = i});
	}
	SymbolPushScope(&t);
	for (u32 i = 5; i < 15; i++) {
		StrID insert = {.idx = i, .gen = 1};
		SymbolInsert(&t, insert, (SymbolEntry){.type = S_VAR, .data.d.i = i * 2});
	}

	for (u32 i = 0; i < 15; i++) {
		StrID insert = {.idx = i, .gen = 1};
		SymbolEntry e = SymbolGet(&t, insert);
		print(stdout, "\t(%d %d %d)\n", i, e.type, e.data.d.i);
	}

	SymbolPopScope(&t);

	for (u32 i = 0; i < 15; i++) {
		StrID insert = {.idx = i, .gen = 1};
		SymbolEntry e = SymbolGet(&t, insert);
		print(stdout, "\t(%d %d %d)\n", i, e.type, e.data.d.i);
	}

    SymbolPopScope(&t);

    Free(t.mem, t.scopes, t.cap * sizeof(SymbolMap));

	return 0;
}
