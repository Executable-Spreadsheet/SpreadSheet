#include <libparasheet/lib_internal.h>
#include <stdint.h>
#include <stdio.h>
#include <util/util.h>

int main() {

	SpreadSheet s = {
		.mem = GlobalAllocatorCreate(),
	};

    for (u32 i = 0; i < 100; i++) {
        SheetBlockInsert(&s, (v2u){i, 0}, UINT32_MAX);
    }

    print(stdout, "Table:\n");
    for (u32 i = 0; i < s.cap; i++) {
        print(stdout, "\t(%d %d)\n", s.keys[i].x, s.keys[i].y);
    }

	for (u32 i = 0; i < 30; i++) {
        SheetBlockDelete(&s, (v2u){i + 2, 0});
	}

	print(stdout, "poolsize: %d\n", s.bcap);
	print(stdout, "freelist:");
	for (u32 i = 0; i < s.fsize; i++) {
		print(stdout, " %d", s.freestatus[i]);
	}
	print(stdout, "\n");

    print(stdout, "Table:\n");
    for (u32 i = 0; i < s.cap; i++) {
        print(stdout, "\t(%d %d)\n", s.keys[i].x, s.keys[i].y);
    }

    for (u32 i = 32; i < 100; i++) {
        assert(SheetBlockGet(&s, (v2u){i, 0}) != UINT32_MAX);
    }

    SpreadSheetClear(&s);

    for (u32 i = 32; i < 100; i++) {
        assert(SheetBlockGet(&s, (v2u){i, 0}) == UINT32_MAX);
    }

	SpreadSheetFree(&s);
}
