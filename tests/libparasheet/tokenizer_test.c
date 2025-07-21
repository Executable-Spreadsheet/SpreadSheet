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
		"if (x + 42 == 25) { let numba : float = 2.1; return numba; } else"
		"{ return \"Hold on a second,\\nthis is a different data type!\"; }";

	// Verify token types
	TokenList* tokens = Tokenize(input, &stringTable, allocator);
	assert(ConsumeToken(tokens)->type == TOKEN_KEYWORD_IF);
	assert(ConsumeToken(tokens)->type == TOKEN_CHAR_OPEN_PAREN);
	assert(ConsumeToken(tokens)->type == TOKEN_ID);
	assert(ConsumeToken(tokens)->type == TOKEN_CHAR_PLUS);
	assert(ConsumeToken(tokens)->type == TOKEN_LITERAL_INT);
	assert(ConsumeToken(tokens)->type == TOKEN_DOUBLECHAR_EQUALS_EQUALS);
	assert(ConsumeToken(tokens)->type == TOKEN_LITERAL_INT);
	assert(ConsumeToken(tokens)->type == TOKEN_CHAR_CLOSE_PAREN);
	assert(ConsumeToken(tokens)->type == TOKEN_CHAR_OPEN_BRACE);
	assert(ConsumeToken(tokens)->type == TOKEN_KEYWORD_LET);
	assert(ConsumeToken(tokens)->type == TOKEN_ID);
	assert(ConsumeToken(tokens)->type == TOKEN_CHAR_COLON);
	assert(ConsumeToken(tokens)->type == TOKEN_KEYWORD_FLOAT);
	assert(ConsumeToken(tokens)->type == TOKEN_CHAR_EQUALS);
	assert(ConsumeToken(tokens)->type == TOKEN_LITERAL_FLOAT);
	assert(ConsumeToken(tokens)->type == TOKEN_CHAR_SEMICOLON);
	assert(ConsumeToken(tokens)->type == TOKEN_KEYWORD_RETURN);
	assert(ConsumeToken(tokens)->type == TOKEN_ID);
	assert(ConsumeToken(tokens)->type == TOKEN_CHAR_SEMICOLON);
	assert(ConsumeToken(tokens)->type == TOKEN_CHAR_CLOSE_BRACE);
	assert(ConsumeToken(tokens)->type == TOKEN_KEYWORD_ELSE);
	assert(ConsumeToken(tokens)->type == TOKEN_CHAR_OPEN_BRACE);
	assert(ConsumeToken(tokens)->type == TOKEN_KEYWORD_RETURN);
	assert(ConsumeToken(tokens)->type == TOKEN_LITERAL_STRING);
	assert(ConsumeToken(tokens)->type == TOKEN_CHAR_SEMICOLON);
	assert(ConsumeToken(tokens)->type == TOKEN_CHAR_CLOSE_BRACE);

	// Verify literals
	assert(tokens->tokens[4].data.i == 42);
	assert(tokens->tokens[6].data.i == 25);
	// My float parsing is apparently different from how C does it, so I need a
	// threshold
	assert(ABS(tokens->tokens[14].data.f - 2.1) < 0.0001);

	SString output = StringGet(&stringTable, tokens->tokens[23].data.s);
	assert(
		SStrCmp(output,
				sstring("Hold on a second,\nthis is a different data type!")) ==
		0);

	// Free string literal because the string interning doesn't strdup, so the
	// Tokenizer leaks memory
	Free(allocator, output.data, output.size * sizeof(output.data[0]));

	StringFree(&stringTable);

	DestroyTokenList(&tokens);
	return 0;
}
