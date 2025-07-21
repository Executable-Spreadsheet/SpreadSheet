#ifndef PS_TOKENIZER_TYPES_H
#define PS_TOKENIZER_TYPES_H

#include "libparasheet/lib_internal.h"
#include <util/util.h>

typedef enum TokenType : u32 {
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

	// Character Tokens
	TOKEN_CHAR_PLUS,
	TOKEN_CHAR_MINUS,
	TOKEN_CHAR_ASTERISK,
	TOKEN_CHAR_SLASH,
	TOKEN_CHAR_OCTOTHORPE,
	TOKEN_CHAR_COLON,

	// Grouping Tokens
	TOKEN_CHAR_OPEN_PAREN,
	TOKEN_CHAR_CLOSE_PAREN,
	TOKEN_CHAR_OPEN_BRACKET,
	TOKEN_CHAR_CLOSE_BRACKET,
	TOKEN_CHAR_OPEN_BRACE,
	TOKEN_CHAR_CLOSE_BRACE,

	// Other Special Characters
	TOKEN_CHAR_EQUALS,
	TOKEN_CHAR_COMMMA,
	TOKEN_CHAR_SEMICOLON,
	TOKEN_CHAR_GREATER_THAN,
	TOKEN_CHAR_LESS_THAN,
	TOKEN_CHAR_EXCLAMATION,

	TOKEN_DOUBLECHAR_EQUALS_EQUALS,
	TOKEN_DOUBLECHAR_LESS_EQUALS,
	TOKEN_DOUBLECHAR_GREATER_EQUALS,
	TOKEN_DOUBLECHAR_EXCLAMATION_EQUALS,
	TOKEN_DOUBLECHAR_AMPERSAND_AMPERSAND,
	TOKEN_DOUBLECHAR_PIPE_PIPE,

	// Enum Size
	TOKEN_TYPE_ENUM_SIZE
} TokenType;

union TokenData {
	StrID s;
	u32 i;
	f32 f;
};

typedef struct Token {
  TokenType type;
  StrID sourceString;
  union TokenData data;
  u32 lineNumber;
} Token;

typedef struct TokenList {
	Allocator mem;
	Token* tokens;
	u32 head;
	u32 size;
	u32 capacity;
} TokenList;

TokenList* CreateTokenList(Allocator allocator);

void PushToken(TokenList* tokenList, TokenType type, StrID sourceString, u32 lineNumber);

void PushTokenLiteral(TokenList* tokenList, TokenType type, StrID string, u32 lineNumber, union TokenData data);

Token* PopTokenDangerous(TokenList* tokenList);

Token* ConsumeToken(TokenList* tokenList);
void UnconsumeToken(TokenList* tokenList);

SString getTokenErrorString(TokenType type);

void DestroyTokenList(TokenList** tokenList);
AST* BuildASTFromTokens(TokenList* tokens, Allocator allocator);

#endif
