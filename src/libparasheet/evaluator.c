#include <libparasheet/lib_internal.h>
#include <libparasheet/evaluator.h>
#include <libparasheet/tokenizer.h>
#include <libparasheet/tokenizer_types.h>


#include <util/util.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

//TEMP
extern Allocator GlobalAllocator;


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

    AST* tree = (AST*)(uintptr_t)(srcCell->d.index);
    if (!tree) return;

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
    CellValue result = evaluateNode(&tree, add, &dummyCtx);

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

    result = evaluateNode(&tree, mul, &dummyCtx);
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

    result = evaluateNode(&tree, ifNode, &dummyCtx);
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

    result = evaluateNode(&tree, seq, &dummyCtx);
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
