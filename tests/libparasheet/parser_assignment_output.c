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

	testString("= let x : int;\n x = 2;\n return x + 2;", &s, allocator);

    StringFree(&s);
	return 0;
}
