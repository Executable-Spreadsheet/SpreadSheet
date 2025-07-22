#include <libparasheet/evaluator.h>
#include <libparasheet/lib_internal.h>
#include <util/util.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper: create and initialize a spreadsheet
SpreadSheet makeSheet(Allocator alloc) {
    SpreadSheet sheet = {
        .mem = alloc,
        .values = NULL,
        .keys = NULL,
        .size = 0,
        .tomb = 0,
        .cap = 0,
        .blockpool = NULL,
        .freestatus = NULL,
        .bsize = 0,
        .fsize = 0,
        .bcap = 0,
    };
    return sheet;
}

// Test: 2 + 3 = 5
void test_addition() {
    Allocator alloc = AllocatorCreateDefault();
    SpreadSheet src = makeSheet(alloc);
    SpreadSheet out = makeSheet(alloc);

    // Build AST manually
    AST* tree = PushStruct(&alloc, AST);
    tree->mem = alloc;
    tree->size = 0;
    tree->cap = 4;
    tree->nodes = PushArray(&alloc, ASTNode, tree->cap);

    u32 lhs = ASTPush(tree);
    tree->nodes[lhs] = (ASTNode){ .op = AST_INT_LITERAL, .data.i = 2 };

    u32 rhs = ASTPush(tree);
    tree->nodes[rhs] = (ASTNode){ .op = AST_INT_LITERAL, .data.i = 3 };

    u32 root = ASTPush(tree);
    tree->nodes[root] = (ASTNode){ .op = AST_ADD, .lchild = lhs, .mchild = rhs };

    // Place AST as a formula in src spreadsheet
    CellValue formula = { .t = CT_CODE, .d.index = (u32)(uintptr_t)tree };
    SpreadSheetSetCell(&src, (v2u){0, 0}, formula);

    // Evaluate
    EvaluateCell(&src, NULL, &out, 0, 0);

    CellValue* result = SpreadSheetGetCell(&out, (v2u){0, 0});
    assert(result->t == CT_INT);
    assert(result->d.i == 5);

    printf("[PASS] 2 + 3 = %d\n", result->d.i);
}

// Entry point
int main() {
    test_addition();
    printf("All evaluator tests passed.\n");
    return 0;
}
