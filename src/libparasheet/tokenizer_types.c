#include "util/util.h"
#include <libparasheet/tokenizer_types.h>

void PushToken(TokenList* tokenList, TokenType type, SString string,
			   u32 lineNumber) {
	PushTokenID(tokenList, type, string, 0, lineNumber);
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

void PushTokenID(TokenList* tokenList, TokenType type, SString string,
				 u32 lineNumber, u32 symbolTableIndex) {
	if (tokenList->size == tokenList->capacity) {
		tokenList->tokens = Realloc(tokenList->mem, tokenList->tokens,
									tokenList->capacity * sizeof(Token),
									tokenList->capacity * 2 * sizeof(Token));

		tokenList->capacity *= 2;
	}

	tokenList->tokens[tokenList->size].type = type;
	tokenList->tokens[tokenList->size].string = string;
	tokenList->tokens[tokenList->size].symbolTableIndex = symbolTableIndex;
	tokenList->tokens[tokenList->size].lineNumber = lineNumber;
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
