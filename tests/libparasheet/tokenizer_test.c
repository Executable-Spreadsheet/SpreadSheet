#include <libparasheet/lib_internal.h>
#include <libparasheet/tokenizer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/util.h>

int main() {
	Allocator allocator = GlobalAllocatorCreate();

	StringTable stringTable = {.mem = allocator};

	const char* input =
		"if (x + 42 == 25) { let numba : float = 21; return numba; } else"
		"{ return \"Hold on a second,\\n, this is a different data type!\"; }";

	TokenList* tokens = Tokenize(input, &stringTable, allocator);
	assert(tokens->tokens[0].type == TOKEN_KEYWORD_IF);
	// I'll wait for the consume token stuff to be merged

	DestroyTokenList(&tokens);
	return 0;
}
