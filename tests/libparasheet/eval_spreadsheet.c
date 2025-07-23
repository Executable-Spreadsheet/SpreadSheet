#include <libparasheet/tokenizer.h>
#include <libparasheet/lib_internal.h>
#include <libparasheet/evaluator.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/util.h>

#define EPS UINT32_MAX

#define PRINT_TOKENS 1
#define PRINT_AST 1

void testString(char* input, StringTable* table, Allocator allocator){
	SpreadSheet testSheet = {
		.mem = allocator,
	};
}

// Optional helper
int main() {
	Allocator allocator = GlobalAllocatorCreate();
    StringTable s = {
        .mem = allocator,
    };
	testString("=2+2;", &s, allocator);
	testString("=2+2;3+3;", &s, allocator);
	testString("=let x : int = 2; let y : int = 2; x + y;", &s, allocator);
	testString("=let x : int = 3; let y: int = 4; 2 * (x + y);", &s, allocator);

    StringFree(&s);
	return 0;
}
