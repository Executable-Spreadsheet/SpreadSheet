#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "lib_internal.h"
#include "util/util.h"

//EvalContext definition
typedef struct EvalContext {
    Allocator mem;
    SpreadSheet* srcSheet;
    SpreadSheet* inSheet;
    SpreadSheet* outSheet;
    StringTable* str;
    SymbolTable* table;
    u32 currentX;
    u32 currentY;
} EvalContext;

// Evaluates a single cell by walking its AST and computing the result.
// srcSheet: the original source sheet where the ASTs are stored
// inSheet:  the input spreadsheet (read-only values for references)
// outSheet: the destination to store computed values
void EvaluateCell(EvalContext ctx);

CellValue evaluateNode(AST* tree, u32 index, EvalContext ctx);


#endif // EVALUATOR_H
