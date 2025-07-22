#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <libparasheet/lib_internal.h>
#include <libparasheet/evaluator.h>

// Declare the functions to test
CellValue evaluateNode(AST* tree, u32 index, struct EvalContext* ctx);

// Stub: This must match what your evaluateNode expects
CellValue evaluateLiteral(ASTNode* node) {
    CellValue v;
    if (node->op == AST_INT_LITERAL) {
        v.t = CT_INT;
        v.d.i = node->data.i;
    } else if (node->op == AST_FLOAT_LITERAL) {
        v.t = CT_FLOAT;
        v.d.f = node->data.f;
    } else {
        fprintf(stderr, "Invalid literal\n");
        exit(1);
    }
    return v;
}

int main() {
    // Fake AST for: 2 + 3
    ASTNode nodes[3];

    // Left operand: 2
    nodes[0].op = AST_INT_LITERAL;
    nodes[0].data.i = 2;

    // Right operand: 3
    nodes[1].op = AST_INT_LITERAL;
    nodes[1].data.i = 3;

    // Root node: 2 + 3
    nodes[2].op = AST_ADD;
    nodes[2].lchild = 0;
    nodes[2].mchild = 1;

    AST tree = {
        .nodes = nodes,
        .size = 3
    };

    EvalContext ctx = {
        .tree = &tree
    };

    CellValue result = evaluateNode(&tree, 2, &ctx);
    assert(result.t == CT_INT);
    assert(result.d.i == 5);
    printf("[PASS] 2 + 3 = %d\n", result.d.i);

    return 0;
}
