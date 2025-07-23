#include <libparasheet/tokenizer.h>
#include <libparasheet/lib_internal.h>
#include <libparasheet/evaluator.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/util.h>

#define EPS UINT32_MAX

#define PRINT_TOKENS 1
#define PRINT_AST 1

void testString(char* input, StringTable* table, Allocator allocator){
	TokenList* tokens = Tokenize(input, table, allocator);
	AST ast = BuildASTFromTokens(tokens, table, allocator);

    SymbolTable sym = {
        .mem = GlobalAllocatorCreate(),
    };
    SymbolPushScope(&sym);

	EvalContext evalContext = (EvalContext){
        .table = &sym,
        .str = &table,

    };
	if (PRINT_TOKENS){
		for (int j = 0; j < tokens->size; j++){
			log("tok: %s", getTokenErrorString(tokens->tokens[j].type));
		}
	}
	assert(ast.size > 0);
	if (PRINT_AST){
		ASTPrint(stdout, &ast);
	}
	CellValue evaluation = evaluateNode(&ast, ast.size - 1, evalContext);
	log("eval type %d: %d/%f", evaluation.t, evaluation.d.i, evaluation.d.f);

    while (sym.size){
        SymbolPopScope(&sym);
    }
    Free(sym.mem, sym.scopes, sym.cap * sizeof(SymbolMap));

	ASTFree(&ast);
	DestroyTokenList(&tokens);
}

// Optional helper
int main() {
	Allocator allocator = GlobalAllocatorCreate();
    StringTable s = {
        .mem = allocator,
    };
	testString("=2+2;", &s, allocator);
	testString("=2+2;3+3;", &s, allocator);
	testString("=let x : int = 2; let y : int = 2; x + y;", &s, allocator);
	testString("=let x : int = 3; let y: int = 4; 2 * (x + y);", &s, allocator);

    StringFree(&s);
	return 0;
}
