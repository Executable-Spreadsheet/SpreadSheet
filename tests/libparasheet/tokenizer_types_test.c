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
	SString example1 = sstring("hello");
	SString example2 = sstring("world");

	PushToken(list, TOKEN_ID, example1, 0);
	PushTokenID(list, TOKEN_LITERAL_STRING, example2, 42, 0);

	// Print tokens
	printf("TokenList Size: %u\n", list->size);
	for (u32 i = 0; i < list->size; i++) {
		Token* t = &list->tokens[i];
		printf("Token %u: type=%u, str='%.*s', symbolIndex=%u\n", i, t->type,
			   t->string.size, t->string.data, t->symbolTableIndex);
	}

	// Pop the last token
	Token* popped = PopTokenDangerous(list);
	if (popped) {
		printf("Popped token: type=%u, str='%.*s', symbolIndex=%u\n",
			   popped->type, popped->string.size, popped->string.data,
			   popped->symbolTableIndex);
	}

	// Cleanup
	DestroyTokenList(&list);
	return 0;
}
