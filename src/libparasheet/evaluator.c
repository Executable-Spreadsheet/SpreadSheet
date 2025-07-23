#include <libparasheet/evaluator.h>
#include <libparasheet/lib_internal.h>
#include <libparasheet/tokenizer.h>
#include <libparasheet/tokenizer_types.h>

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/util.h>

// Forward declarations
static CellValue evaluateLiteral(ASTNode* node);
static CellValue evaluateBinaryOp(AST* tree, ASTNode* node, EvalContext ctx);
static CellValue evaluateCellRef(AST* tree, ASTNode* node, EvalContext ctx);

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
static CellValue evaluateBinaryOp(AST* tree, ASTNode* node, EvalContext ctx) {
	CellValue lhs = evaluateNode(tree, node->lchild, ctx);
	CellValue rhs = evaluateNode(tree, node->mchild, ctx);
	CellValue result;

	bool isFloat = (lhs.t == CT_FLOAT || rhs.t == CT_FLOAT);
	result.t = isFloat ? CT_FLOAT : CT_INT;

	float lf = lhs.t == CT_FLOAT ? lhs.d.f : (float)lhs.d.i;
	float rf = rhs.t == CT_FLOAT ? rhs.d.f : (float)rhs.d.i;

	switch (node->op) {
	case AST_ADD:
		if (isFloat)
			result.d.f = lf + rf;
		else
			result.d.i = lhs.d.i + rhs.d.i;
		break;

	case AST_SUB:
		if (isFloat)
			result.d.f = lf - rf;
		else
			result.d.i = lhs.d.i - rhs.d.i;
		break;

	case AST_MUL:
		if (isFloat)
			result.d.f = lf * rf;
		else
			result.d.i = lhs.d.i * rhs.d.i;
		break;

	case AST_DIV:
		if (rf == 0.0f) {
			fprintf(stderr, "Divide by zero\n");
			exit(1);
		}
		if (isFloat)
			result.d.f = lf / rf;
		else
			result.d.i = lhs.d.i / rhs.d.i;
		break;

	default:
		fprintf(stderr, "Unknown binary op: %u\n", node->op);
		exit(1);
	}

	return result;
}

// Core evaluator function that dispatches based on AST node type
CellValue evaluateNode(AST* tree, u32 index, EvalContext ctx) {
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
		CellValue a = evaluateNode(tree, node->lchild, ctx);
		CellValue b = evaluateNode(tree, node->mchild, ctx);
		if (b.t == CT_EMPTY)
			return a;
		return b;
	}

	case AST_IF_ELSE: {
		CellValue condition = evaluateNode(tree, node->lchild, ctx);
		bool cond = (condition.t == CT_INT)
						? (condition.d.i != 0)
						: (condition.t == CT_FLOAT && condition.d.f != 0.0f);

		if (cond)
			return evaluateNode(tree, node->mchild, ctx); // then branch
		else
			return evaluateNode(tree, node->rchild, ctx); // else branch
	}

	case AST_RETURN:
		return evaluateNode(tree, node->lchild, ctx);

	case AST_WHILE:
	case AST_FOR:
		fprintf(stderr, "Control flow '%u' not implemented yet\n", node->op);
		exit(1);

	case AST_DECLARE_VARIABLE: {
		SymbolEntry e = {
			.type = S_VAR,
		};

		if (node->vt == V_INT)
			e.data.t = CT_INT;
		if (node->vt == V_FLOAT)
			e.data.t = CT_FLOAT;

		log("declare: %s", StringGet(ctx.str, node->data.s));
		SymbolInsert(ctx.table, node->data.s, e);
		return e.data;
	} break;
	case AST_ASSIGN_VALUE: {
		CellValue lhs = evaluateNode(tree, node->lchild, ctx);
		CellValue rhs = evaluateNode(tree, node->mchild, ctx);

		StrID var = ASTGet(tree, node->lchild).data.s;
		log("assign: %d", StringGet(ctx.str, var));
		SymbolEntry e = SymbolGet(ctx.table, var);
		if (e.type == S_INVALID) {
			err("inavalid name");
			panic();
		}

		if (e.data.t == rhs.t) {
			e.data = rhs;
			SymbolInsert(ctx.table, var, e);
			return e.data;
		}

		if (e.data.t == CT_INT) {
			e.data.d.i = (i32)rhs.d.f;
		} else if (e.data.t == CT_FLOAT) {
			e.data.d.f = (f32)rhs.d.i;
		}

		return e.data;
	} break;
	case AST_ID: {

		StrID var = node->data.s;
		SymbolEntry e = SymbolGet(ctx.table, var);
		if (e.type == S_INVALID) {
			err("inavalid name");
			panic();
		}
		return e.data;
	}
	case AST_SCOPE_BEGIN: {
		SymbolPushScope(ctx.table);
		return (CellValue){0};
	} break;
	case AST_SCOPE_END: {
		SymbolPopScope(ctx.table);
		return (CellValue){0};
	} break;
	case AST_INT_TYPE:
	case AST_FLOAT_TYPE:
		fprintf(stderr,
				"Variable/Type logic not implemented yet in evaluator\n");
		exit(1);

	default:
		err("Unhandled AST node type: %d\n", node->op);
		panic();
	}
}

// clarise TODO: add error checking, somehow?
// maybe create a new cell value of "#ERROR!" lol
// Evaluates a single cell in the spreadsheet, recursively if needed
void EvaluateCell(EvalContext ctx) {

	SpreadSheet* srcSheet = ctx.srcSheet;
	SpreadSheet* inSheet = ctx.inSheet;
	SpreadSheet* outSheet = ctx.outSheet;
	u32 cellX = ctx.currentX;
	u32 cellY = ctx.currentY;
	StringTable* strTable = ctx.str;
	Allocator allocator = ctx.mem;

	v2u pos = {cellX, cellY};

	CellValue* sourceCell = SpreadSheetGetCell(srcSheet, pos);
	// Eli's code checks for numbers at entry into sheet from file.
	// cell knows if it is a number (int/float) or a string. parse string.

	EvalContext evalContext = ctx;
	SymbolPushScope(ctx.table);

	switch (sourceCell->t) {
	case CT_INT:
	case CT_FLOAT:
		SpreadSheetSetCell(outSheet, pos, *sourceCell);
		break;
	default: {
		SString input = StringGet(strTable, sourceCell->d.index);

		// is a string
		// invoke the tokenizer
		TokenList* tokens =
			Tokenize((const char*)input.data, strTable, allocator);
		// run the parser on the tokens
		AST ast = BuildASTFromTokens(tokens, strTable, allocator);
		// run the evaluator on the ast
		CellValue result = evaluateNode(&ast, ast.size - 1, evalContext);
		SpreadSheetSetCell(outSheet, pos, result);
		// error checking
		DestroyTokenList(&tokens);
		ASTFree(&ast);
	} break;
	}
	/*
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

	// 🔧 Reconstruct AST* from StrID
	uintptr_t raw = ((uintptr_t)srcCell->d.index.gen << 32) |
	srcCell->d.index.idx; AST* tree = (AST*)raw;

	if (!tree) return;

	u32 rootIndex = tree->size - 1;  // Root is always the last node
	*/
	// EvalContext ctx = {
	//     .srcSheet = srcSheet,
	//     .inSheet = inSheet,
	//     .outSheet = outSheet,
	//     .currentX = cellX,
	//     .currentY = cellY,
	// };

	//// ✅ Declare and assign result before use
	// CellValue result = evaluateNode(tree, rootIndex, &ctx);
	// SpreadSheetSetCell(outSheet, pos, result);
}

static CellValue evaluateCellRef(AST* tree, ASTNode* node, EvalContext ctx) {
	ctx.currentX = ASTGet(tree, node->lchild).data.i;
	ctx.currentY = ASTGet(tree, node->mchild).data.i;

	// Recursively evaluate the referenced cell
	EvaluateCell(ctx);

	// Return the already-computed result
	return *SpreadSheetGetCell(ctx.outSheet, (v2u){ctx.currentX, ctx.currentY});
}
