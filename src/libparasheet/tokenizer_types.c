#include "libparasheet/lib_internal.h"
#include <libparasheet/tokenizer_types.h>

// TokenList* CreateTokenList(Allocator allocator) {
// 	TokenList* tokenList = Alloc(allocator, sizeof(TokenList));
// 	tokenList->mem = allocator;
// 	tokenList->size = 0;

// 	tokenList->capacity = 2;
// 	tokenList->tokens = Alloc(allocator, tokenList->capacity * sizeof(Token));

// 	return tokenList;
// }

void PushToken(TokenList* tokenList, TokenType type, StrID sourceString,
			   u32 lineNumber) {
	union TokenData noData = {.i = 0};
	PushTokenLiteral(tokenList, type, sourceString, lineNumber, noData);
}

TokenList* CreateTokenList(Allocator allocator) {
	TokenList* tokenList = Alloc(allocator, sizeof(TokenList));
	tokenList->mem = allocator;
	tokenList->size = 0;

	tokenList->head = 0;
  
	tokenList->capacity = 2;
	tokenList->tokens = Alloc(allocator, tokenList->capacity * sizeof(Token));
	return tokenList;
}

void PushTokenLiteral(TokenList* tokenList, TokenType type, StrID sourceString,
					  u32 lineNumber, union TokenData data) {
	if (tokenList->size == tokenList->capacity) {
		tokenList->tokens = Realloc(tokenList->mem, tokenList->tokens,
									tokenList->capacity * sizeof(Token),
									tokenList->capacity * 2 * sizeof(Token));

		tokenList->capacity *= 2;
	}

	tokenList->tokens[tokenList->size].type = type;
	tokenList->tokens[tokenList->size].sourceString = sourceString;
	tokenList->tokens[tokenList->size].lineNumber = lineNumber;
	tokenList->tokens[tokenList->size].data = data;
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

Token* PeekToken(TokenList* tokenList){
	return &(tokenList->tokens[tokenList->head]);
}

SString tokenErrorStrings[] = {
	sstring("INVALID"),		sstring("variable name"), sstring("\"if\""),
	sstring("\"else\""),	sstring("\"return\""),	  sstring("\"let\""),
	sstring("\"while\""),	sstring("\"for\""),		  sstring("\"int\""),
	sstring("\"float\""),	sstring("\"string\""),	  sstring("\"cell\""),
	sstring("int literal"), sstring("float literal"), sstring("\"string\""),
	sstring("\"+\""),		sstring("\"-\""),		  sstring("\"*\""),
	sstring("\"/\""),		sstring("\"#\""),		  sstring("\"#\""),
	sstring("\"(\""),		sstring("\")\""),		  sstring("\"[\""),
	sstring("\"]\""),		sstring("\"{\""),		  sstring("\"}\""),
	sstring("\"=\""),		sstring("\",\""),		  sstring("\";\""),
	sstring("INVALID")};

SString getTokenErrorString(TokenType type) { return tokenErrorStrings[type]; }

void DestroyTokenList(TokenList** tokenListPtr) {
	Free((*tokenListPtr)->mem, (*tokenListPtr)->tokens,
		 (*tokenListPtr)->capacity);
	Free((*tokenListPtr)->mem, *tokenListPtr, sizeof(*tokenListPtr));
	*tokenListPtr = NULL;
}
