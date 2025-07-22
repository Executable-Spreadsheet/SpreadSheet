#include <libparasheet/lib_internal.h>
#include <libparasheet/evaluator.h>
#include <libparasheet/tokenizer.h>
#include <libparasheet/tokenizer_types.h>


#include <util/util.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


// Forward declarations
static CellValue evaluateLiteral(ASTNode* node);
static CellValue evaluateBinaryOp(AST* tree, ASTNode* node, EvalContext* ctx);
static CellValue evaluateNode(AST* tree, u32 index, EvalContext* ctx);
static CellValue evaluateCellRef(AST* tree, ASTNode* node, EvalContext* ctx);
void EvaluateCell(SpreadSheet* srcSheet, SpreadSheet* inSheet, SpreadSheet* outSheet, u32 cellX, u32 cellY);

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

// Evaluates a binary operator node: +, -, *, /
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

// Evaluates a FOR loop: initializer; condition; body (may include increment)
static CellValue evaluateForLoop(AST* tree, ASTNode* node, EvalContext* ctx) {
    // Run the initialization (lchild)
    evaluateNode(tree, node->lchild, ctx);

    CellValue lastResult = { .t = CT_EMPTY };

    while (true) {
        // Evaluate the loop condition (mchild)
        CellValue cond = evaluateNode(tree, node->mchild, ctx);
        bool isTrue = (cond.t == CT_INT && cond.d.i != 0) ||
                      (cond.t == CT_FLOAT && cond.d.f != 0.0f);

        if (!isTrue)
            break;

        // Execute loop body (rchild), which should include any increment
        lastResult = evaluateNode(tree, node->rchild, ctx);
    }

    return lastResult;
}


// Core evaluator function that dispatches based on AST node type
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

        case AST_SEQ: {
            // Evaluate lchild and mchild sequentially; return result of mchild
            evaluateNode(tree, node->lchild, ctx);
            return evaluateNode(tree, node->mchild, ctx);
        }

        case AST_IF_ELSE: {
            CellValue condition = evaluateNode(tree, node->lchild, ctx);
            bool cond = (condition.t == CT_INT) ? (condition.d.i != 0) :
                        (condition.t == CT_FLOAT && condition.d.f != 0.0f);

            if (cond)
                return evaluateNode(tree, node->mchild, ctx);  // then branch
            else
                return evaluateNode(tree, node->rchild, ctx);  // else branch
        }

        case AST_RETURN:
            return evaluateNode(tree, node->lchild, ctx);

        case AST_WHILE:
        case AST_FOR:
            fprintf(stderr, "Control flow '%u' not implemented yet\n", node->op);
            exit(1);

        case AST_DECLARE_VARIABLE:
        case AST_ASSIGN_VALUE:
        case AST_ID:
        case AST_INT_TYPE:
        case AST_FLOAT_TYPE:
            fprintf(stderr, "Variable/Type logic not implemented yet in evaluator\n");
            exit(1);

        default:
            fprintf(stderr, "Unhandled AST node type: %u\n", node->op);
            exit(1);
    }
}

// Evaluates a single cell in the spreadsheet, recursively if needed
void EvaluateCell(SpreadSheet* srcSheet, SpreadSheet* inSheet, SpreadSheet* outSheet, u32 cellX, u32 cellY) {
    v2u pos = { cellX, cellY };

    // If already evaluated, return
    CellValue* outCell = SpreadSheetGetCell(outSheet, pos);
    if (outCell->t != CT_EMPTY) return;

    // Get formula/code from source sheet
    CellValue* srcCell = SpreadSheetGetCell(srcSheet, pos);

    // If it's not code (e.g. it's a literal value), just copy it directly
    if (srcCell->t != CT_CODE) {
        *outCell = *srcCell;
        return;
    }

    // Extract the StrID that points to the formula string
    StrID strId = *((StrID*)&srcCell->d.index);  // Assumes d.index holds packed StrID — verify this is safe
    SString formula = StringGet(&stringTable, strId);  // Get the actual formula string

    // Tokenize the formula string
    TokenList* tokens = Tokenize(formula.data, &stringTable, GlobalAllocator);
    if (!tokens) {
        fprintf(stderr, "Tokenization failed at (%u, %u)\n", cellX, cellY);
        return;
    }

    // Parse tokens into an AST
    AST* tree = Parse(tokens);
    if (!tree) {
        fprintf(stderr, "Parsing failed at (%u, %u)\n", cellX, cellY);
        DestroyTokenList(&tokens);
        return;
    }

    u32 rootIndex = tree->size - 1;  // Root is always the last node

    EvalContext ctx = {
        .srcSheet = srcSheet,
        .inSheet = inSheet,
        .outSheet = outSheet,
        .currentX = cellX,
        .currentY = cellY,
    };

    CellValue result = evaluateNode(tree, rootIndex, &ctx);
    SpreadSheetSetCell(outSheet, pos, result);  // Store result in output sheet

}


static CellValue evaluateCellRef(AST* tree, ASTNode* node, EvalContext* ctx) {
    u32 x = ASTGet(tree, node->lchild).data.i;
    u32 y = ASTGet(tree, node->mchild).data.i;

    // Recursively evaluate the referenced cell
    EvaluateCell(ctx->srcSheet, ctx->inSheet, ctx->outSheet, x, y);

    // Return the already-computed result
    return *SpreadSheetGetCell(ctx->outSheet, (v2u){x, y});
}
