#include <libparasheet/lib_internal.h>
#include <libparasheet/tokenizer.h>
#include <stdio.h>
#include <util/util.h>

AST* BuildASTFromTokens(TokenList* tokens, Allocator allocator);

// TODO(QUINCY): Write a proper test for the parser

int main(int argc, char** argv) {
	logfile = fopen("/dev/null", "w");

	Allocator globalAlloc = GlobalAllocatorCreate();

	const char* exampleCode = "=(a : int, b : int)\n"
							  "if (12)\n"
							  "return (1 * 2) + 1;\n"
							  "else\n"
							  "while (12)\n"
							  "return 12;\n"
							  "for (i, 12);\n"
							  "return 12;\n";

	TokenList* tokens = Tokenize(exampleCode, globalAlloc);
	AST* ast = BuildASTFromTokens(tokens, globalAlloc);
	if (ast && ast->size > 0) {
		ASTPrint(stdout, ast);
	}
}
