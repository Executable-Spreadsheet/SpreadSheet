#include <libparasheet/tokenizer_types.h>

// TokenList* CreateTokenList(Allocator allocator) {
// 	TokenList* tokenList = Alloc(allocator, sizeof(TokenList));
// 	tokenList->mem = allocator;
// 	tokenList->size = 0;

// 	tokenList->capacity = 2;
// 	tokenList->tokens = Alloc(allocator, tokenList->capacity * sizeof(Token));

// 	return tokenList;
// }

void PushToken(TokenList* tokenList, TokenType type, SString string) {
	PushTokenID(tokenList, type, string, 0);
}

// void PushTokenID(TokenList* tokenList, TokenType type, SString string,
// 				 u32 symbolTableIndex) {
// 	if (tokenList->size == tokenList->capacity) {
// 		tokenList->tokens =
// 			Realloc(tokenList->mem, tokenList->tokens, tokenList->capacity,
// 					tokenList->capacity * 2);
// 		tokenList->capacity = tokenList->capacity * 2;
// 	}

// 	tokenList->tokens[tokenList->size].type = type;
// 	tokenList->tokens[tokenList->size].string = string;
// 	tokenList->tokens[tokenList->size].symbolTableIndex = symbolTableIndex;

// 	tokenList->size += 1;
// }

TokenList* CreateTokenList(Allocator allocator) {
	TokenList* tokenList = Alloc(allocator, sizeof(TokenList));
	tokenList->mem = allocator;
	tokenList->size = 0;
	tokenList->capacity = 2;
	tokenList->tokens = Alloc(allocator, tokenList->capacity * sizeof(Token));
	return tokenList;
}

void PushTokenID(TokenList* tokenList, TokenType type, SString string, u32 symbolTableIndex) {
	if (tokenList->size == tokenList->capacity) {
		//FIXED: Pass sizes in bytes, not counts
		tokenList->tokens = Realloc(
			tokenList->mem,
			tokenList->tokens,
			tokenList->capacity * sizeof(Token),
			tokenList->capacity * 2 * sizeof(Token));

		tokenList->capacity *= 2;
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
	tokenList->size -= 1;
	Token* top = &(tokenList->tokens[tokenList->size]);	
	return top;
}

void DestroyTokenList(TokenList** tokenListPtr) {
	Free((*tokenListPtr)->mem, (*tokenListPtr)->tokens,
		 (*tokenListPtr)->capacity);
	Free((*tokenListPtr)->mem, *tokenListPtr, sizeof(*tokenListPtr));
	*tokenListPtr = NULL;
}
