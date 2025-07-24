#include <libparasheet/evaluator.h>
#include <libparasheet/lib_internal.h>
#include <libparasheet/tokenizer.h>
#include <libparasheet/tokenizer_types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/util.h>

#define EPS UINT32_MAX

#define PRINT_TOKENS 1
#define PRINT_AST 1

void printAST(char* input, StringTable* table, Allocator allocator) {
	TokenList* tokens = Tokenize(input, table, allocator);
	AST ast = BuildASTFromTokens(tokens, table, allocator);
	for (int j = 0; j < tokens->size; j++) {
		warn("tok: %s\n", getTokenErrorString(tokens->tokens[j].type));
	}
	if (ast.size > 0) {
		ASTPrint(stdout, &ast);
		for (int i = 0; i < ast.size - 1; i++) {
			ASTNode node = ast.nodes[i];
			assert(node.lchild == EPS || node.lchild < i);
			assert(node.mchild == EPS || node.mchild < i);
			assert(node.rchild == EPS || node.rchild < i);
		}
	} else {
		assert(0);
	}
	ASTFree(&ast);
	DestroyTokenList(&tokens);
}

void testString(char* input, CellValue expected, StringTable* strTable,
				Allocator allocator) {
	SpreadSheet testSheet = {
		.mem = allocator,
	};
	SpreadSheet outputSheet = {
		.mem = allocator,
	};
	SymbolTable cymbals = {
		.mem = allocator,
	};

	StrID code = StringAddS(strTable, sstring(input));
	SpreadSheetSetCell(&testSheet, (v2u){.x = 0, .y = 0},
					   (CellValue){.t = CT_TEXT, .d.index = code});

	SymbolPushScope(&cymbals);

	printAST(input, strTable, allocator);

	EvaluateCell((EvalContext){
		.srcSheet = &testSheet,
		.inSheet = &outputSheet,
		.outSheet = &outputSheet,
		.currentX = 0,
		.currentY = 0,
		.mem = allocator,
		.str = strTable,
		.table = &cymbals,
	});

	CellValue* outputValue =
		SpreadSheetGetCell(&outputSheet, (v2u){.x = 0, .y = 0});

	// assert cell value is equal to expected
	assert(outputValue->t == expected.t && outputValue->d.i == expected.d.i);

	while (cymbals.size) {
		SymbolPopScope(&cymbals);
	}
	Free(allocator, cymbals.scopes, cymbals.cap * sizeof(cymbals.scopes[0]));

	SpreadSheetFree(&testSheet);
	SpreadSheetFree(&outputSheet);

	StringDel(strTable, code);
}

// Optional helper
int main() {
	Allocator allocator = GlobalAllocatorCreate();
	StringTable s = {
		.mem = allocator,
	};
	// testString("=2+2;", (CellValue){.t = CT_INT, .d.i = 4}, &s, allocator);
	// testString("=2+2;3+3;", (CellValue){.t = CT_INT, .d.i = 6}, &s,
	// allocator); testString("=let x : int = 2; let y : int = 2; x + y;",
	// 		   (CellValue){.t = CT_INT, .d.i = 4}, &s, allocator);
	// testString("=let x : int = 3; let y: int = 4; 2 * (x + y);",
	// 		   (CellValue){.t = CT_INT, .d.i = 14}, &s, allocator);
	testString("=\nlet curr : int = 0;\n"
			"let i : int = 3;\nwhile (i) {\n"
			"    curr = curr + [0, 3 - i];\n"
			"    i = i - 1;\n}}\nreturn curr;",
			(CellValue){.t = CT_INT, .d.i = 0}, &s, allocator);

	StringFree(&s);
	return 0;
}
