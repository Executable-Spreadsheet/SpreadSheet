#include <assert.h>
#include <libparasheet/evaluator.h>
#include <libparasheet/lib_internal.h>
#include <util/util.h>

int main() {
	Allocator AllieO_O = GlobalAllocatorCreate();

	StringTable shaun = {
		.mem = AllieO_O,
	};

	SpreadSheet steve = {
		.mem = AllieO_O,
	};
	SpreadSheet stephan = {
		.mem = AllieO_O,
	};
	SymbolTable sid = {
		.mem = AllieO_O,
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
							   .mem = AllieO_O,
							   .str = &shaun,
							   .table = &sid});

	CellValue* stephanson = SpreadSheetGetCell(&stephan, (v2u){.x = 1, .y = 1});

	assert(stephanson->d.i == 2);

	return 0;
}
