#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <libparasheet/lib_internal.h>
#include <libparasheet/evaluator.h>

// Declare the internal function
CellValue evaluateLiteral(ASTNode* node);

void test_int_literal() {
    ASTNode node;
    node.op = AST_INT_LITERAL;
    node.data.i = 42;

    CellValue val = evaluateLiteral(&node);
    assert(val.t == CT_INT);
    assert(val.d.i == 42);
    printf("[PASS] Integer literal: %d\n", val.d.i);
}

void test_float_literal() {
    ASTNode node;
    node.op = AST_FLOAT_LITERAL;
    node.data.f = 3.14f;

    CellValue val = evaluateLiteral(&node);
    assert(val.t == CT_FLOAT);
    assert(val.d.f == 3.14f);
    printf("[PASS] Float literal: %f\n", val.d.f);
}

int main() {
    test_int_literal();
    test_float_literal();
    printf("All evaluateLiteral tests passed.\n");
    return 0;
}
