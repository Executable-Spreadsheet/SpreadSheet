
#include <libparasheet/lib_internal.h>
#include <util/util.h>

int main() {


    StringTable str = {
        .mem = GlobalAllocatorCreate(),
    };

    u32 test = StringAdd(&str, (i8*)"test");
    StringAdd(&str, (i8*)"test2");
    u32 e = StringAdd(&str, (i8*)"e");
    StringAdd(&str, (i8*)"test3");
    StringAdd(&str, (i8*)"test4");

    print(stdout, "Table: (%d %d)\n", str.size, str.cap);
    for (u32 i = 0; i < str.cap; i++) {
        print(stdout, "\t(%d, %d)\n", str.meta[i], str.vals[i]);
    }

    StringDel(&str, e);
    StringDel(&str, test);
    StringAdd(&str, (i8*)"test5");

    print(stdout, "Table:\n");
    for (u32 i = 0; i < str.cap; i++) {
        print(stdout, "\t(%d, %d)\n", str.meta[i], str.vals[i]);
    }

    StringFree(&str);

    return 0;
}
