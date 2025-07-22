#include <libparasheet/evaluator.h>
#include <libparasheet/lib_internal.h>
#include <util/util.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

void testEvaluateNode() {
    Allocator alloc = GlobalAllocatorCreate();

    AST tree = {
        .mem = alloc,
        .nodes = NULL,
        .cap = 0,
        .size = 0
    };

    EvalContext dummyCtx = {0};
    CellValue result;

    printf("== Running evaluateNode tests ==\n");

    // Test 1: 7 + 3
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

    result = evaluateNode(&tree, add, &dummyCtx);
    printf("Test 1: 7 + 3 = %d\n", result.d.i);
    assert(result.t == CT_INT && result.d.i == 10);

    // Test 2: 4.0 * 2
    u32 f = ASTPush(&tree);
    tree.nodes[f].op = AST_FLOAT_LITERAL;
    tree.nodes[f].data.f = 4.0f;

    u32 i = ASTPush(&tree);
    tree.nodes[i].op = AST_INT_LITERAL;
    tree.nodes[i].data.i = 2;

    u32 mul = ASTPush(&tree);
    tree.nodes[mul].op = AST_MUL;
    tree.nodes[mul].lchild = f;
    tree.nodes[mul].mchild = i;

    result = evaluateNode(&tree, mul, &dummyCtx);
    float diff = result.d.f - 8.0f;
    if (diff < 0) diff = -diff;
    printf("Test 2: 4.0 * 2 = %.2f\n", result.d.f);
    assert(result.t == CT_FLOAT && diff < 0.001f);

    // Test 3: if (1) { 10 } else { 20 }
    u32 cond = ASTPush(&tree);
    tree.nodes[cond].op = AST_INT_LITERAL;
    tree.nodes[cond].data.i = 1;

    u32 tval = ASTPush(&tree);
    tree.nodes[tval].op = AST_INT_LITERAL;
    tree.nodes[tval].data.i = 10;

    u32 eval = ASTPush(&tree);
    tree.nodes[eval].op = AST_INT_LITERAL;
    tree.nodes[eval].data.i = 20;

    u32 ifnode = ASTPush(&tree);
    tree.nodes[ifnode].op = AST_IF_ELSE;
    tree.nodes[ifnode].lchild = cond;
    tree.nodes[ifnode].mchild = tval;
    tree.nodes[ifnode].rchild = eval;

    result = evaluateNode(&tree, ifnode, &dummyCtx);
    printf("Test 3: if (1) {10} else {20} = %d\n", result.d.i);
    assert(result.t == CT_INT && result.d.i == 10);

    // Test 4: seq { 5; 6 } returns 6
    u32 s1 = ASTPush(&tree);
    tree.nodes[s1].op = AST_INT_LITERAL;
    tree.nodes[s1].data.i = 5;

    u32 s2 = ASTPush(&tree);
    tree.nodes[s2].op = AST_INT_LITERAL;
    tree.nodes[s2].data.i = 6;

    u32 seq = ASTPush(&tree);
    tree.nodes[seq].op = AST_SEQ;
    tree.nodes[seq].lchild = s1;
    tree.nodes[seq].mchild = s2;

    result = evaluateNode(&tree, seq, &dummyCtx);
    printf("Test 4: seq {5; 6} = %d\n", result.d.i);
    assert(result.t == CT_INT && result.d.i == 6);

        // Test 5: if (0) { 100 } else { 200 }
    u32 cond0 = ASTPush(&tree);
    tree.nodes[cond0].op = AST_INT_LITERAL;
    tree.nodes[cond0].data.i = 0;

    u32 t100 = ASTPush(&tree);
    tree.nodes[t100].op = AST_INT_LITERAL;
    tree.nodes[t100].data.i = 100;

    u32 e200 = ASTPush(&tree);
    tree.nodes[e200].op = AST_INT_LITERAL;
    tree.nodes[e200].data.i = 200;

    u32 if0 = ASTPush(&tree);
    tree.nodes[if0].op = AST_IF_ELSE;
    tree.nodes[if0].lchild = cond0;
    tree.nodes[if0].mchild = t100;
    tree.nodes[if0].rchild = e200;

    result = evaluateNode(&tree, if0, &dummyCtx);
    printf("Test 5: if (0) {100} else {200} = %d\n", result.d.i);
    assert(result.t == CT_INT && result.d.i == 200);

    // Test 6: if (1.0f) { 1; 2; 3 } else { 9; 9; 9 }
    u32 condf = ASTPush(&tree);
    tree.nodes[condf].op = AST_FLOAT_LITERAL;
    tree.nodes[condf].data.f = 1.0f;

    // Then branch: 1; 2; 3
    u32 t1 = ASTPush(&tree);
    tree.nodes[t1].op = AST_INT_LITERAL;
    tree.nodes[t1].data.i = 1;

    u32 t2 = ASTPush(&tree);
    tree.nodes[t2].op = AST_INT_LITERAL;
    tree.nodes[t2].data.i = 2;

    u32 t3 = ASTPush(&tree);
    tree.nodes[t3].op = AST_INT_LITERAL;
    tree.nodes[t3].data.i = 3;

    u32 seqt12 = ASTPush(&tree);
    tree.nodes[seqt12].op = AST_SEQ;
    tree.nodes[seqt12].lchild = t1;
    tree.nodes[seqt12].mchild = t2;

    u32 seqt123 = ASTPush(&tree);
    tree.nodes[seqt123].op = AST_SEQ;
    tree.nodes[seqt123].lchild = seqt12;
    tree.nodes[seqt123].mchild = t3;

    // Else: 9; 9; 9
    u32 e1 = ASTPush(&tree);
    tree.nodes[e1].op = AST_INT_LITERAL;
    tree.nodes[e1].data.i = 9;

    u32 e2 = ASTPush(&tree);
    tree.nodes[e2].op = AST_INT_LITERAL;
    tree.nodes[e2].data.i = 9;

    u32 e3 = ASTPush(&tree);
    tree.nodes[e3].op = AST_INT_LITERAL;
    tree.nodes[e3].data.i = 9;

    u32 seqe12 = ASTPush(&tree);
    tree.nodes[seqe12].op = AST_SEQ;
    tree.nodes[seqe12].lchild = e1;
    tree.nodes[seqe12].mchild = e2;

    u32 seqe123 = ASTPush(&tree);
    tree.nodes[seqe123].op = AST_SEQ;
    tree.nodes[seqe123].lchild = seqe12;
    tree.nodes[seqe123].mchild = e3;

    u32 if_f = ASTPush(&tree);
    tree.nodes[if_f].op = AST_IF_ELSE;
    tree.nodes[if_f].lchild = condf;
    tree.nodes[if_f].mchild = seqt123;
    tree.nodes[if_f].rchild = seqe123;

    result = evaluateNode(&tree, if_f, &dummyCtx);
    printf("Test 6: if (1.0f) {1;2;3} else {9;9;9} = %d\n", result.d.i);
    assert(result.t == CT_INT && result.d.i == 3);

    // Test 7: nested if-else (if 1 then (if 0 then 5 else 6) else 7) => 6
    u32 cond1a = ASTPush(&tree);
    tree.nodes[cond1a].op = AST_INT_LITERAL;
    tree.nodes[cond1a].data.i = 1;

    u32 cond0a = ASTPush(&tree);
    tree.nodes[cond0a].op = AST_INT_LITERAL;
    tree.nodes[cond0a].data.i = 0;

    u32 innerThen = ASTPush(&tree);
    tree.nodes[innerThen].op = AST_INT_LITERAL;
    tree.nodes[innerThen].data.i = 5;

    u32 innerElse = ASTPush(&tree);
    tree.nodes[innerElse].op = AST_INT_LITERAL;
    tree.nodes[innerElse].data.i = 6;

    u32 innerIf = ASTPush(&tree);
    tree.nodes[innerIf].op = AST_IF_ELSE;
    tree.nodes[innerIf].lchild = cond0a;
    tree.nodes[innerIf].mchild = innerThen;
    tree.nodes[innerIf].rchild = innerElse;

    u32 outerElse = ASTPush(&tree);
    tree.nodes[outerElse].op = AST_INT_LITERAL;
    tree.nodes[outerElse].data.i = 7;

    u32 outerIf = ASTPush(&tree);
    tree.nodes[outerIf].op = AST_IF_ELSE;
    tree.nodes[outerIf].lchild = cond1a;
    tree.nodes[outerIf].mchild = innerIf;
    tree.nodes[outerIf].rchild = outerElse;

    result = evaluateNode(&tree, outerIf, &dummyCtx);
    printf("Test 7: nested if = %d\n", result.d.i);
    assert(result.t == CT_INT && result.d.i == 6);

    // Test 8: if (1) { return float 3.5 } else { return int 2 }
    u32 condMix = ASTPush(&tree);
    tree.nodes[condMix].op = AST_INT_LITERAL;
    tree.nodes[condMix].data.i = 1;

    u32 floatThen = ASTPush(&tree);
    tree.nodes[floatThen].op = AST_FLOAT_LITERAL;
    tree.nodes[floatThen].data.f = 3.5f;

    u32 intElse = ASTPush(&tree);
    tree.nodes[intElse].op = AST_INT_LITERAL;
    tree.nodes[intElse].data.i = 2;

    u32 ifMixed = ASTPush(&tree);
    tree.nodes[ifMixed].op = AST_IF_ELSE;
    tree.nodes[ifMixed].lchild = condMix;
    tree.nodes[ifMixed].mchild = floatThen;
    tree.nodes[ifMixed].rchild = intElse;

    result = evaluateNode(&tree, ifMixed, &dummyCtx);
    printf("Test 8: if (1) float 3.5 else 2 = %.2f\n", result.d.f);
    assert(result.t == CT_FLOAT && result.d.f == 3.5f);

    
    ASTFree(&tree);
    printf("✅ All evaluateNode tests passed!\n");

}

int main() {
    testEvaluateNode();
    return 0;
}
