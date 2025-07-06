#include <util/util.h>

int main() {

    Allocator g = GlobalAllocatorCreate();
    Allocator a = StackAllocatorCreate(g, sizeof(int) * 100);


    int* s = Alloc(a, sizeof(int) * 10);
    for (u32 i = 0; i < 10; i++) {
        s[i] = i;
    }
    for (u32 i = 0; i < 10; i++) {
        log("s: %d", s[i]);
    }

    s = Realloc(a, s, sizeof(int) * 10, sizeof(int) * 15);

    for (u32 i = 0; i < 15; i++) {
        s[i] = i;
    }
    for (u32 i = 0; i < 15; i++) {
        log("s: %d", s[i]);
    }


    Free(a, s, sizeof(int) * 15);
    StackAllocatorReset(&a);


    u64* l = Alloc(a, sizeof(u64) * 10);

    for (u32 i = 0; i < 10; i++) {
        l[i] = i * 10;
    }
    for (u32 i = 0; i < 10; i++) {
        log("s: %lX", l[i]);
    }


    StackAllocatorDestroy(&a);
}

