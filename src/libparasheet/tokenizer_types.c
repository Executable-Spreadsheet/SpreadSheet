#include <libparasheet/tokenizer_types.h>

TokenList* CreateTokenList(Allocator allocator) {
	TokenList* tokenList = Alloc(allocator, sizeof(TokenList));
	tokenList->mem = allocator;
	tokenList->size = 0;

	tokenList->head = 0;

	tokenList->capacity = 2;
	tokenList->tokens = Alloc(allocator, tokenList->capacity * sizeof(Token));

	return tokenList;
}

void PushToken(TokenList* tokenList, TokenType type, SString string) {
	PushTokenID(type, string, 0);
}

void PushTokenID(TokenList* tokenList, TokenType type, SString string,
				 u32 symbolTableIndex) {
	if (tokenList->size == tokenList->capacity) {
		tokenList->tokens =
			Realloc(tokenList->mem, tokenList->tokens, tokenList->capacity,
					tokenList->capacity * 2);
		tokenList->capacity = tokenList->capacity * 2;
	}

	tokenList->tokens[tokenList->size].type = type;
	tokenList->tokens[tokenList->size].string = string;
	tokenList->tokens[tokenList->size].symbolTableIndex = symbolTableIndex;

	tokenList->size += 1;
}

// WARNING: This memory *will be overwritten* if you push after popping.
Token* PopTokenDangerous(TokenList* tokenList) {
	if (tokenList->size == 0) {
		return NULL;
	}
	Token* top = &(tokenList->tokens[tokenList->size]);
	tokenList->size -= 1;
	return top;
}

Token* ConsumeToken(TokenList* tokenList) {
	if (tokenList->head == tokenList->size) {
		return NULL;
	}
	Token* consumed = &(tokenList->tokens[tokenList->head]);
	tokenList->head += 1;
	return consumed;
}
void UnconsumeToken(TokenList* tokenList) {
	if (tokenList->head == 0) {
		return;
	}
	tokenList->head -= 1;
}

void DestroyTokenList(TokenList** tokenListPtr) {
	Free((*tokenListPtr)->mem, (*tokenListPtr)->tokens,
		 (*tokenListPtr)->capacity);
	Free((*tokenListPtr)->mem, *tokenListPtr, sizeof(*tokenListPtr));
	*tokenListPtr = NULL;
}
