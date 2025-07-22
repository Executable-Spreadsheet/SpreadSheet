#include <libparasheet/lib_internal.h>
#include <libparasheet/evaluator.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct EvalContext {
    SpreadSheet* srcSheet;
    SpreadSheet* inSheet;
    SpreadSheet* outSheet;
    u32 currentX;
    u32 currentY;
    AST* tree;
} EvalContext;


static CellValue evaluateBinaryOp(AST* tree, ASTNode* node, EvalContext* ctx) {
    CellValue lhs = evaluateNode(tree, node->lchild, ctx);
    CellValue rhs = evaluateNode(tree, node->mchild, ctx);
    CellValue result;

    bool isFloat = (lhs.t == CT_FLOAT || rhs.t == CT_FLOAT);
    result.t = isFloat ? CT_FLOAT : CT_INT;

    float lf = lhs.t == CT_FLOAT ? lhs.d.f : (float)lhs.d.i;
    float rf = rhs.t == CT_FLOAT ? rhs.d.f : (float)rhs.d.i;

    switch (node->op) {
        case AST_ADD:
            result.d.f = isFloat ? lf + rf : lhs.d.i + rhs.d.i;
            break;
        case AST_SUB:
            result.d.f = isFloat ? lf - rf : lhs.d.i - rhs.d.i;
            break;
        case AST_MUL:
            result.d.f = isFloat ? lf * rf : lhs.d.i * rhs.d.i;
            break;
        case AST_DIV:
            if (rf == 0.0f) {
                fprintf(stderr, "Divide by zero\n");
                exit(1);
            }
            result.d.f = isFloat ? lf / rf : lhs.d.i / rhs.d.i;
            break;
        default:
            fprintf(stderr, "Unknown binary op: %u\n", node->op);
            exit(1);
    }

    return result;
}

static CellValue evaluateNode(AST* tree, u32 index, EvalContext* ctx) {
    ASTNode* node = &ASTGet(tree, index);

    switch (node->op) {
        case AST_INT_LITERAL:
        case AST_FLOAT_LITERAL:
            return evaluateLiteral(node);

        case AST_GET_CELL_REF:
            return evaluateCellRef(tree, node, ctx);

        case AST_ADD:
        case AST_SUB:
        case AST_MUL:
        case AST_DIV:
            return evaluateBinaryOp(tree, node, ctx);

        default:
            fprintf(stderr, "Unhandled AST node type: %u\n", node->op);
            exit(1);
    }
}