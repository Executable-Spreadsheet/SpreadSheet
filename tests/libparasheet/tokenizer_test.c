#include "libparasheet/tokenizer_types.h"
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
	Token* curr = ConsumeToken(tokens);
	assert(curr->type == TOKEN_KEYWORD_IF);
	assert(SStrCmp(StringGet(&stringTable, curr->sourceString),
				   sstring("if")) == 0);
	curr = ConsumeToken(tokens);
	assert(curr->type == TOKEN_CHAR_OPEN_PAREN);
	assert(SStrCmp(StringGet(&stringTable, curr->sourceString), sstring("(")) ==
		   0);
	curr = ConsumeToken(tokens);
	assert(curr->type == TOKEN_ID);
	assert(SStrCmp(StringGet(&stringTable, curr->sourceString), sstring("x")) ==
		   0);
	curr = ConsumeToken(tokens);
	assert(curr->type == TOKEN_CHAR_PLUS);
	assert(SStrCmp(StringGet(&stringTable, curr->sourceString), sstring("+")) ==
		   0);
	curr = ConsumeToken(tokens);
	assert(curr->type == TOKEN_LITERAL_INT);
	assert(SStrCmp(StringGet(&stringTable, curr->sourceString),
				   sstring("42")) == 0);
	curr = ConsumeToken(tokens);
	assert(curr->type == TOKEN_DOUBLECHAR_EQUALS_EQUALS);
	assert(SStrCmp(StringGet(&stringTable, curr->sourceString),
				   sstring("==")) == 0);
	curr = ConsumeToken(tokens);
	assert(curr->type == TOKEN_LITERAL_INT);
	assert(SStrCmp(StringGet(&stringTable, curr->sourceString),
				   sstring("25")) == 0);
	curr = ConsumeToken(tokens);
	assert(curr->type == TOKEN_CHAR_CLOSE_PAREN);
	assert(SStrCmp(StringGet(&stringTable, curr->sourceString), sstring(")")) ==
		   0);
	curr = ConsumeToken(tokens);
	assert(curr->type == TOKEN_CHAR_OPEN_BRACE);
	assert(SStrCmp(StringGet(&stringTable, curr->sourceString), sstring("{")) ==
		   0);
	curr = ConsumeToken(tokens);
	assert(curr->type == TOKEN_KEYWORD_LET);
	assert(SStrCmp(StringGet(&stringTable, curr->sourceString),
				   sstring("let")) == 0);
	curr = ConsumeToken(tokens);
	assert(curr->type == TOKEN_ID);
	assert(SStrCmp(StringGet(&stringTable, curr->sourceString),
				   sstring("numba")) == 0);
	curr = ConsumeToken(tokens);
	assert(curr->type == TOKEN_CHAR_COLON);
	assert(SStrCmp(StringGet(&stringTable, curr->sourceString), sstring(":")) ==
		   0);
	curr = ConsumeToken(tokens);
	assert(curr->type == TOKEN_KEYWORD_FLOAT);
	assert(SStrCmp(StringGet(&stringTable, curr->sourceString),
				   sstring("float")) == 0);
	curr = ConsumeToken(tokens);
	assert(curr->type == TOKEN_CHAR_EQUALS);
	assert(SStrCmp(StringGet(&stringTable, curr->sourceString), sstring("=")) ==
		   0);
	curr = ConsumeToken(tokens);
	assert(curr->type == TOKEN_LITERAL_FLOAT);
	assert(SStrCmp(StringGet(&stringTable, curr->sourceString),
				   sstring("2.1")) == 0);
	curr = ConsumeToken(tokens);
	assert(curr->type == TOKEN_CHAR_SEMICOLON);
	assert(SStrCmp(StringGet(&stringTable, curr->sourceString), sstring(";")) ==
		   0);
	curr = ConsumeToken(tokens);
	assert(curr->type == TOKEN_KEYWORD_RETURN);
	assert(SStrCmp(StringGet(&stringTable, curr->sourceString),
				   sstring("return")) == 0);
	curr = ConsumeToken(tokens);
	assert(curr->type == TOKEN_ID);
	assert(SStrCmp(StringGet(&stringTable, curr->sourceString),
				   sstring("numba")) == 0);
	curr = ConsumeToken(tokens);
	assert(curr->type == TOKEN_CHAR_SEMICOLON);
	assert(SStrCmp(StringGet(&stringTable, curr->sourceString), sstring(";")) ==
		   0);
	curr = ConsumeToken(tokens);
	assert(curr->type == TOKEN_CHAR_CLOSE_BRACE);
	assert(SStrCmp(StringGet(&stringTable, curr->sourceString), sstring("}")) ==
		   0);
	curr = ConsumeToken(tokens);
	assert(curr->type == TOKEN_KEYWORD_ELSE);
	assert(SStrCmp(StringGet(&stringTable, curr->sourceString),
				   sstring("else")) == 0);
	curr = ConsumeToken(tokens);
	assert(curr->type == TOKEN_CHAR_OPEN_BRACE);
	assert(SStrCmp(StringGet(&stringTable, curr->sourceString), sstring("{")) ==
		   0);
	curr = ConsumeToken(tokens);
	assert(curr->type == TOKEN_KEYWORD_RETURN);
	assert(SStrCmp(StringGet(&stringTable, curr->sourceString),
				   sstring("return")) == 0);
	curr = ConsumeToken(tokens);
	assert(curr->type == TOKEN_LITERAL_STRING);
	assert(SStrCmp(
			   StringGet(&stringTable, curr->sourceString),
			   sstring(
				   "\"Hold on a second,\\nthis is a different data type!\"")) ==
		   0);
	curr = ConsumeToken(tokens);
	assert(curr->type == TOKEN_CHAR_SEMICOLON);
	assert(SStrCmp(StringGet(&stringTable, curr->sourceString), sstring(";")) ==
		   0);
	curr = ConsumeToken(tokens);
	assert(curr->type == TOKEN_CHAR_CLOSE_BRACE);
	assert(SStrCmp(StringGet(&stringTable, curr->sourceString), sstring("}")) ==
		   0);

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
