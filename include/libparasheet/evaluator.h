#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "lib_internal.h"

// Local EvalContext definition for testing
typedef struct EvalContext {
    SpreadSheet* srcSheet;
    SpreadSheet* inSheet;
    SpreadSheet* outSheet;
    u32 currentX;
    u32 currentY;
} EvalContext;

// Evaluates a single cell by walking its AST and computing the result.
// srcSheet: the original source sheet where the ASTs are stored
// inSheet:  the input spreadsheet (read-only values for references)
// outSheet: the destination to store computed values
void EvaluateCell(SpreadSheet* srcSheet, SpreadSheet* inSheet, SpreadSheet* outSheet, u32 cellX, u32 cellY);

#endif // EVALUATOR_H
