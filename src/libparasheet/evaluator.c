#include <libparasheet/lib_internal.h>
#include <libparasheet/evaluator.h>
#include <util/util.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


// Forward declarations
static CellValue evaluateNode(AST* tree, u32 index, EvalContext* ctx);
static CellValue evaluateBinaryOp(AST* tree, ASTNode* node, EvalContext* ctx);
static CellValue evaluateLiteral(ASTNode* node);
static CellValue evaluateCellRef(AST* tree, ASTNode* node, EvalContext* ctx);

// Evaluator logic
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
            if (isFloat) result.d.f = lf + rf;
            else result.d.i = lhs.d.i + rhs.d.i;
            break;

        case AST_SUB:
            if (isFloat) result.d.f = lf - rf;
            else result.d.i = lhs.d.i - rhs.d.i;
            break;

        case AST_MUL:
            if (isFloat) result.d.f = lf * rf;
            else result.d.i = lhs.d.i * rhs.d.i;
            break;

        case AST_DIV:
            if (rf == 0.0f) {
                fprintf(stderr, "Divide by zero\n");
                exit(1);
            }
            if (isFloat) result.d.f = lf / rf;
            else result.d.i = lhs.d.i / rhs.d.i;
            break;

        default:
            fprintf(stderr, "Unknown binary op: %u\n", node->op);
            exit(1);
    }

    return result;
}


CellValue evaluateNode(AST* tree, u32 index, EvalContext* ctx) {
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

void EvaluateCell(SpreadSheet* srcSheet, SpreadSheet* inSheet, SpreadSheet* outSheet, u32 cellX, u32 cellY) {

}

static CellValue evaluateCellRef(AST* tree, ASTNode* node, EvalContext* ctx) {
    u32 x = ASTGet(tree, node->lchild).data.i;
    u32 y = ASTGet(tree, node->mchild).data.i;

    // Recursively evaluate the referenced cell
    EvaluateCell(ctx->srcSheet, ctx->inSheet, ctx->outSheet, x, y);

    // Return the already-computed result
    return *SpreadSheetGetCell(ctx->outSheet, (v2u){x, y});
}
