#include <libparasheet/evaluator.h>
#include <libparasheet/lib_internal.h>
#include <util/util.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test function for evaluateNode()

void testEvaluateNode() {
    Allocator alloc = GlobalAllocatorCreate();  

    AST tree = {
        .mem = alloc,
        .nodes = NULL,
        .cap = 0,
        .size = 0
    };

    printf("== Running evaluateNode tests ==\n");

    // -------- Test 1: 7 + 3 --------
    u32 lhs = ASTPush(&tree);
    tree.nodes[lhs].op = AST_INT_LITERAL;
    tree.nodes[lhs].data.i = 7;

    u32 rhs = ASTPush(&tree);
    tree.nodes[rhs].op = AST_INT_LITERAL;
    tree.nodes[rhs].data.i = 3;

    u32 add = ASTPush(&tree);
    tree.nodes[add].op = AST_ADD;
    tree.nodes[add].lchild = lhs;
    tree.nodes[add].mchild = rhs;

    EvalContext dummyCtx = {0};
    CellValue result = evaluateNode(&tree, add, dummyCtx);

    printf("Test 1 (7 + 3): %d\n", result.d.i);
    assert(result.t == CT_INT && result.d.i == 10);

    // -------- Test 2: 4.0 * 2 --------
    u32 fl = ASTPush(&tree);
    tree.nodes[fl].op = AST_FLOAT_LITERAL;
    tree.nodes[fl].data.f = 4.0f;

    u32 i2 = ASTPush(&tree);
    tree.nodes[i2].op = AST_INT_LITERAL;
    tree.nodes[i2].data.i = 2;

    u32 mul = ASTPush(&tree);
    tree.nodes[mul].op = AST_MUL;
    tree.nodes[mul].lchild = fl;
    tree.nodes[mul].mchild = i2;

    result = evaluateNode(&tree, mul, dummyCtx);
    printf("Test 2 (4.0 * 2): %.2f\n", result.d.f);
    float diff = result.d.f - 8.0f;
    if (diff < 0) diff = -diff;
    assert(result.t == CT_FLOAT && diff < 0.001f);
    
    // -------- Test 3: IF (1) { 10 } ELSE { 20 } --------
    u32 cond = ASTPush(&tree);
    tree.nodes[cond].op = AST_INT_LITERAL;
    tree.nodes[cond].data.i = 1;

    u32 thenVal = ASTPush(&tree);
    tree.nodes[thenVal].op = AST_INT_LITERAL;
    tree.nodes[thenVal].data.i = 10;

    u32 elseVal = ASTPush(&tree);
    tree.nodes[elseVal].op = AST_INT_LITERAL;
    tree.nodes[elseVal].data.i = 20;

    u32 ifNode = ASTPush(&tree);
    tree.nodes[ifNode].op = AST_IF_ELSE;
    tree.nodes[ifNode].lchild = cond;
    tree.nodes[ifNode].mchild = thenVal;
    tree.nodes[ifNode].rchild = elseVal;

    result = evaluateNode(&tree, ifNode, dummyCtx);
    printf("Test 3 (if 1 then 10 else 20): %d\n", result.d.i);
    assert(result.t == CT_INT && result.d.i == 10);

    // -------- Test 4: Sequence { 5; 6 } returns 6 --------
    u32 seq1 = ASTPush(&tree);
    tree.nodes[seq1].op = AST_INT_LITERAL;
    tree.nodes[seq1].data.i = 5;

    u32 seq2 = ASTPush(&tree);
    tree.nodes[seq2].op = AST_INT_LITERAL;
    tree.nodes[seq2].data.i = 6;

    u32 seq = ASTPush(&tree);
    tree.nodes[seq].op = AST_SEQ;
    tree.nodes[seq].lchild = seq1;
    tree.nodes[seq].mchild = seq2;

    result = evaluateNode(&tree, seq, dummyCtx);
    printf("Test 4 (seq 5; 6): %d\n", result.d.i);
    assert(result.t == CT_INT && result.d.i == 6);

    // -------- Test 5: Division by zero should exit --------
    // NOTE: This test will crash, comment out unless testing error handling
    /*
    u32 zero = ASTPush(&tree);
    tree.nodes[zero].op = AST_INT_LITERAL;
    tree.nodes[zero].data.i = 0;

    u32 div = ASTPush(&tree);
    tree.nodes[div].op = AST_DIV;
    tree.nodes[div].lchild = lhs; // reuse 7
    tree.nodes[div].mchild = zero;

    result = evaluateNode(&tree, div, &dummyCtx);
    */

    ASTFree(&tree);
    printf("✅ All evaluateNode tests passed!\n");
}



// Main function to run test
int main() {
    testEvaluateNode();
    printf("✅ evaluateNode() test passed.\n");
    return 0;
}
