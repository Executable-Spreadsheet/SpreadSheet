#include <libparasheet/tokenizer.h>
#include <libparasheet/lib_internal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/util.h>

#define EPS UINT32_MAX

void testString(char* input, StringTable* table, Allocator allocator){
	TokenList* tokens = Tokenize(input, table, allocator);
	AST ast = BuildASTFromTokens(tokens, table, allocator);
	for (int j = 0; j < tokens->size; j++){
		printf("tok: %s\n", getTokenErrorString(tokens->tokens[j].type));
	}
	if (ast.size > 0){
		ASTPrint(stdout, &ast);
		for (int i = 0; i < ast.size - 1; i++){
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

// Optional helper
int main() {
	Allocator allocator = GlobalAllocatorCreate();
    StringTable s = {
        .mem = allocator,
    };

//	const char* input = "=1 + 2 / 3 * 4;";


//	printf("input string is: %s\n", input);

//	TokenList* tokens = Tokenize(input, &s, allocator);

//	for (u32 i = 0; i < tokens->size; i++) {
//		Token* t = &tokens->tokens[i];
//        SString val = StringGet(&s, t->sourceString);
//		printf("Token %2d: %-20s | Value: %.*s\n", i,
//			   TokenTypeToString(t->type), val.size, val.data);
//	}

//	AST ast = BuildASTFromTokens(tokens, allocator);
//	if (ast.size > 0) {
//		log("AST:");
//		ASTPrint(stdout, &ast);
//	}
	testString("=2+2;", &s, allocator);
	testString("=x + y;", &s, allocator);
	testString("=2 * (x + y);", &s, allocator);

    StringFree(&s);
	return 0;
}
