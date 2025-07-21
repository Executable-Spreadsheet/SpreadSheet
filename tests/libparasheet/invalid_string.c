#include <libparasheet/lib_internal.h>
#include <util/util.h>

int main() {


    StringTable str = {
        .mem = GlobalAllocatorCreate(),
    };

    StrID test = StringAdd(&str, (i8*)"test");

    print(stdout, "Table: (%d %d)\n", str.size, str.cap);
    for (u32 i = 0; i < str.cap; i++) {
        print(stdout, "\t(%d, %d)\n", str.meta[i], str.vals[i]);
    }

    SString t = StringDel(&str, test);

    print(stdout, "Table:\n");
    for (u32 i = 0; i < str.cap; i++) {
        print(stdout, "\t(%d, %d)\n", str.meta[i], str.vals[i]);
    }

    SString a = StringGet(&str, test);
    print(stdout, "test: %s\n", t);
    print(stdout, "test: %s\n", a);

    StringFree(&str);

    return 0;
}
