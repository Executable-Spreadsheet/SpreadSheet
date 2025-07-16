#ifndef PS_TOKENIZER_TYPES_H
#define PS_TOKENIZER_TYPES_H

#include <util/util.h>

typedef enum TokenType {
	TOKEN_INVALID = 0, // Mark unintialized Token as invalid

	// Identifier Token, i.e. user declared variables
	// Be sure to also set the symbolTableIndex when setting this to allow the parser to differentiate different tokens
	TOKEN_ID,

	// Keyword Tokens
	TOKEN_KEYWORD_IF,
	TOKEN_KEYWORD_ELSE,
	TOKEN_KEYWORD_RETURN,
	TOKEN_KEYWORD_LET,
	TOKEN_KEYWORD_WHILE,
	TOKEN_KEYWORD_FOR,
	TOKEN_KEYWORD_INT,
	TOKEN_KEYWORD_FLOAT,
	TOKEN_KEYWORD_STRING,
	TOKEN_KEYWORD_CELL,

	// Literal Tokens
	TOKEN_LITERAL_INT,
	TOKEN_LITERAL_FLOAT,
	TOKEN_LITERAL_STRING,

	// Operator Tokens
	TOKEN_OP_PLUS,
	TOKEN_OP_MINUS,
	TOKEN_OP_TIMES,
	TOKEN_OP_DIVIDE,
	TOKEN_OP_OCTOTHORPE,
	TOKEN_OP_COLON,

	// Grouping Tokens
	TOKEN_GROUPING_OPEN_PAREN,
	TOKEN_GROUPING_CLOSE_PAREN,
	TOKEN_GROUPING_OPEN_BRACKET,
	TOKEN_GROUPING_CLOSE_BRACKET,
	TOKEN_GROUPING_OPEN_BRACE,
	TOKEN_GROUPING_CLOSE_BRACE,
} TokenType;

typedef struct Token {
	TokenType type;
	SString string;
	u32 symbolTableIndex;
} Token;

typedef struct TokenList {
	Allocator mem;
	Token* tokens;
	u32 size;
	u32 capacity;
} TokenList;

TokenList* CreateTokenList(Allocator allocator);

void PushToken(TokenList* tokenList, TokenType type, SString string);

void PushTokenID(TokenList* tokenList, TokenType type, SString string, u32 symbolTableIndex);

Token* PopTokenDangerous(TokenList* tokenList);

void DestroyTokenList(TokenList** tokenList);

#endif
