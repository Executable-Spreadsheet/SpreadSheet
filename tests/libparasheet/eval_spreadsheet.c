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

void testString(char* input, CellValue expected,
		StringTable* strTable, SymbolTable* symTable, Allocator allocator){
	SpreadSheet testSheet = {
		.mem = allocator,
	};
	SpreadSheet outputSheet = {
		.mem = allocator,
	};
	
	EvaluateCell((EvalContext){
		.srcSheet = &testSheet,
		.inSheet = &outputSheet,
		.outSheet = &outputSheet,
		.currentX = 0,
		.currentY = 0,
		.mem = allocator,
		.str = strTable,
		.table = symTable,
	});
	
	// assert cell value is equal to expected
	assert();
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
