#include <libparasheet/evaluator.h>
#include <libparasheet/lib_internal.h>
#include <libparasheet/tokenizer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/util.h>

#define EPS UINT32_MAX

#define PRINT_TOKENS 1
#define PRINT_AST 1

void testString(char* input, CellValue expected, StringTable* strTable,
				Allocator allocator) {
	SpreadSheet testSheet = {
		.mem = allocator,
	};
	SpreadSheet outputSheet = {
		.mem = allocator,
	};
	SymbolTable cymbals = {
		.mem = allocator,
	};

	StrID code = StringAddS(strTable, sstring(input));
	SpreadSheetSetCell(&testSheet, (v2u){.x = 0, .y = 0},
					   (CellValue){.t = CT_TEXT, .d.index = code});

	SymbolPushScope(&cymbals);

	EvaluateCell((EvalContext){
		.srcSheet = &testSheet,
		.inSheet = &outputSheet,
		.outSheet = &outputSheet,
		.currentX = 0,
		.currentY = 0,
		.mem = allocator,
		.str = strTable,
		.table = &cymbals,
	});

	CellValue* outputValue =
		SpreadSheetGetCell(&outputSheet, (v2u){.x = 0, .y = 0});

	// assert cell value is equal to expected
	assert(outputValue->t == expected.t && outputValue->d.i == expected.d.i);

	while (cymbals.size) {
		SymbolPopScope(&cymbals);
	}
	Free(allocator, cymbals.scopes, cymbals.cap * sizeof(cymbals.scopes[0]));

	SpreadSheetFree(&testSheet);
	SpreadSheetFree(&outputSheet);

	StringDel(strTable, code);
}

// Optional helper
int main() {
	Allocator allocator = GlobalAllocatorCreate();
	StringTable s = {
		.mem = allocator,
	};
	testString("=2+2;", (CellValue){.t = CT_INT, .d.i = 4}, &s, allocator);
	testString("=2+2;3+3;", (CellValue){.t = CT_INT, .d.i = 6}, &s, allocator);
	testString("=let x : int = 2; let y : int = 2; x + y;",
			   (CellValue){.t = CT_INT, .d.i = 4}, &s, allocator);
	testString("=let x : int = 3; let y: int = 4; 2 * (x + y);",
			   (CellValue){.t = CT_INT, .d.i = 14}, &s, allocator);

	StringFree(&s);
	return 0;
}
