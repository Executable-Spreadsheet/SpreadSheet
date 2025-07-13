#include <libparasheet/lib_internal.h>

int test() {

    SpreadSheet s = {
        .mem = GlobalAllocatorCreate(),
    };

    for (u32 i = 0; i < 3; i++) {
        CellValue v = (CellValue) {
            .t = CT_INT,
            .d = {i},
        };
        SpreadSheetSetCell(&s, (v2u){i, 0}, v);
    }

    for (u32 i = 0; i < 3; i++) {
        CellValue* c = SpreadSheetGetCell(&s, (v2u){i, 0});
        print(stdout, "cell: %p\n", c);
        if (c) {
            print(stdout, "value: %d\n", c->d.i);
            print(stdout, "\tbsize: %d\n", s.bsize);
        }
    }

    for (u32 i = 0; i < 3; i++) {
        SpreadSheetClearCell(&s, (v2u){i, 0});
    }

    print(stdout, "poolsize: %d\n", s.bcap);
    print(stdout, "freelist:");
    for (u32 i = 0; i < s.fsize; i++) {
        print(stdout, " %d", s.freestatus[i]);
    }
    print(stdout, "\n");

    for (u32 i = 0; i < 3; i++) {
        CellValue v = (CellValue) {
            .t = CT_INT,
            .d = {i},
        };
        SpreadSheetSetCell(&s, (v2u){BLOCK_SIZE + i + 1, 0}, v);
    }


    for (u32 i = 0; i < 3; i++) {
        CellValue* c = SpreadSheetGetCell(&s, (v2u){BLOCK_SIZE + i + 1, 0});
        print(stdout, "cell: %p\n", c);
        if (c) {
            print(stdout, "value: %d\n", c->d.i);
            print(stdout, "\tbsize: %d\n", s.bsize);
        }
    }

    print(stdout, "poolsize: %d\n", s.bcap);
    print(stdout, "freelist:");
    for (u32 i = 0; i < s.fsize; i++) {
        print(stdout, " %d", s.freestatus[i]);
    }
    print(stdout, "\n");

    SpreadSheetFree(&s);

    return 0;
}
