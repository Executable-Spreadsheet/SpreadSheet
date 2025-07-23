#include <assert.h>
#include <libparasheet/evaluator.h>
#include <libparasheet/lib_internal.h>
#include <util/util.h>

int main() {
	Allocator AllieOwO = GlobalAllocatorCreate();

	StringTable shaun = {
		.mem = AllieOwO,
	};

	SpreadSheet steve = {
		.mem = AllieOwO,
	};
	SpreadSheet stephan = {
		.mem = AllieOwO,
	};
	SymbolTable sid = {
		.mem = AllieOwO,
	};

	SymbolPushScope(&sid);

	StrID code = StringAddS(&shaun, sstring("=return 1+1;"));
	SpreadSheetSetCell(&steve, (v2u){.x = 1, .y = 1},
					   (CellValue){.t = CT_TEXT, .d.index = code});

	EvaluateCell((EvalContext){.srcSheet = &steve,
							   .inSheet = &stephan,
							   .outSheet = &stephan,
							   .currentX = 1,
							   .currentY = 1,
							   .mem = AllieOwO,
							   .str = &shaun,
							   .table = &sid});

	CellValue* stephanson = SpreadSheetGetCell(&stephan, (v2u){.x = 1, .y = 1});

	assert(stephanson->d.i == 2);

	warn("DONE\n");

	while (sid.size) {
		SymbolPopScope(&sid);
	}
	Free(AllieOwO, sid.scopes, sid.cap * sizeof(sid.scopes[0]));

	SpreadSheetFree(&steve);
	SpreadSheetFree(&stephan);

	StringFree(&shaun);

	return 0;
}
