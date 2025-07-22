#include <libparasheet/tokenizer.h>
#include <libparasheet/lib_internal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/util.h>

#define EPS UINT32_MAX

#define PRINT_TOKENS 0

void testString(char* input, StringTable* table, Allocator allocator){
	TokenList* tokens = Tokenize(input, table, allocator);
	AST ast = BuildASTFromTokens(tokens, table, allocator);
	EvalContext evalContext = (EvalContext){0};
	if (PRINT_TOKENS){
		for (int j = 0; j < tokens->size; j++){
			printf("tok: %s\n", getTokenErrorString(tokens->tokens[j].type));
		}
	}
	assert(ast.size > 0);
	if (PRINT_AST){
		ASTPrint(stdout, &ast);
	}
	CellValue evaluation = evaluateNode(&ast, ast.nodes[ast.size - 1], EvalContext* ctx);
	log("eval type %d: %d/%f", evaluation.t, evaluation.d.i, evaluation.d.f);
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
	testString("=x + y;", &s, allocator);
	testString("=2 * (x + y);", &s, allocator);

    StringFree(&s);
	return 0;
}
