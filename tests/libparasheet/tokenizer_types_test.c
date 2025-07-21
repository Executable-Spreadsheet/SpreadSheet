#include "libparasheet/lib_internal.h"
#include <assert.h>
#include <libparasheet/tokenizer_types.h>
#include <stdio.h>
#include <util/util.h>

int main() {
	Allocator allocator = GlobalAllocatorCreate();

	// Create a token list
	TokenList* list = CreateTokenList(allocator);
	if (!list) {
		printf("Failed to create TokenList\n");
		return 1;
	}

	// Create example tokens
	StrID example1 = {.idx = 1, .gen = 2};
	StrID example2 = {.idx = 3, .gen = 4};

	union TokenData literalExample = {.i = 42};

	// Verify Consume/Unconsume work on empty
	UnconsumeToken(list);
	assert(ConsumeToken(list) == NULL);

	PushToken(list, TOKEN_ID, example1, 1);
	PushTokenLiteral(list, TOKEN_LITERAL_STRING, example2, 2, literalExample);

	// Test Consumption/Unconsumption
	Token* foo = ConsumeToken(list);
	assert(foo->type == TOKEN_ID);
	assert(foo->sourceString.idx == 1);
	assert(foo->sourceString.gen == 2);
	assert(foo->lineNumber == 1);

	Token* bar = ConsumeToken(list);
	assert(bar->type == TOKEN_LITERAL_STRING);
	assert(bar->sourceString.idx == 3);
	assert(bar->sourceString.gen == 4);
	assert(bar->lineNumber == 2);
	assert(bar->data.i == 42);

	UnconsumeToken(list);

	bar = ConsumeToken(list);
	assert(bar->type == TOKEN_LITERAL_STRING);
	assert(bar->sourceString.idx == 3);
	assert(bar->sourceString.gen == 4);
	assert(bar->lineNumber == 2);
	assert(bar->data.i == 42);

	assert(ConsumeToken(list) == NULL);

	// Cleanup
	DestroyTokenList(&list);
	return 0;
}
