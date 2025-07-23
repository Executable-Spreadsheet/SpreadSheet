#include <libparasheet/lib_internal.h>
#include <libparasheet/tokenizer_types.h>
#include <stdbool.h>
#include <string.h>
#include <util/util.h>

static bool is_whitespace(char c) {
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static bool is_alpha(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

// static bool is_digit(char c) {
// 	return c >= '0' && c <= '9';
// }

static bool is_alnum(char c) { return is_alpha(c) || isdigit(c); }

// Internal string compare helper
static bool SString_EqualsCString(const SString* s, const char* cstr) {
	u32 len = 0;
	while (cstr[len])
		len++;
	if (s->size != len)
		return false;
	return memcmp(s->data, cstr, len) == 0;
}

// Create a non-owning substring (view)
SString substr(const char* src, u32 start, u32 end) {
	SString result;
	result.size = end - start;
	result.data = (i8*)(src + start);
	return result;
}

void stringPushCharacter(i8** string, u32* size, u32* capacity,
						 Allocator allocator, char character) {
	if (*size == *capacity) {
		*string =
			Realloc(allocator, *string, (*capacity) * sizeof((*string)[0]),
					(*capacity) * sizeof((*string)[0]) * 2);
		*capacity = (*capacity) * 2;
	}
	(*string)[(*size)] = character;
	*size += 1;
}

void tokenizeStringLiteral(const char* source, Allocator allocator, u32* i,
						   TokenList* tokens, StringTable* table,
						   u32* lineNumber) {
	u32 start = *i;

	u32 newStringSize = 0;
	u32 newStringCapacity = 2;
	i8* newString = Alloc(allocator, sizeof(char) * newStringCapacity);

	bool escaped = false;
	bool endString = false;
	while (!endString) {
		if (source[*i] == '\0') {
			PushToken(tokens, TOKEN_INVALID,
					  StringAddS(table, substr(source, start, *i)),
					  *lineNumber);
			return;
		}
		*i += 1;
		char c = source[*i];
		if (escaped) {
			escaped = false;
			switch (c) {
			case '\\':
				stringPushCharacter(&newString, &newStringSize,
									&newStringCapacity, allocator, '\\');
				break;
			case 'n':
				stringPushCharacter(&newString, &newStringSize,
									&newStringCapacity, allocator, '\n');
				break;
			case 'r':
				stringPushCharacter(&newString, &newStringSize,
									&newStringCapacity, allocator, '\r');
				break;
			case '\"':
				stringPushCharacter(&newString, &newStringSize,
									&newStringCapacity, allocator, '\"');
				break;
			case '\'':
				stringPushCharacter(&newString, &newStringSize,
									&newStringCapacity, allocator, '\'');
				break;
			case '\n':
				*lineNumber += 1;
				break;
			default:
				stringPushCharacter(&newString, &newStringSize,
									&newStringCapacity, allocator, '\\');
				stringPushCharacter(&newString, &newStringSize,
									&newStringCapacity, allocator, c);
				break;
			}
		} else {
			if (c == '\"') {
				endString = true;
				*i += 1;
			} else if (c == '\\') {
				escaped = true;
			} else {
				if (c == '\n') {
					lineNumber += 1;
				}
				stringPushCharacter(&newString, &newStringSize,
									&newStringCapacity, allocator, c);
			}
		}
	}

	newString =
		Realloc(allocator, newString, sizeof(newString[0]) * newStringCapacity,
				sizeof(newString[0]) * newStringSize);
	newStringCapacity = newStringSize;
	SString literalValue = {.data = newString, .size = newStringSize};

	union TokenData data = {.s = StringAddS(table, literalValue)};

	PushTokenLiteral(tokens, TOKEN_LITERAL_STRING,
					 StringAddS(table, substr(source, start, *i)), *lineNumber,
					 data);
}

void tokenizeNumberLiteral(const char* source, u32* i, TokenList* tokens,
						   StringTable* table, u32 lineNumber) {
	TokenType type = TOKEN_LITERAL_INT;
	i32 number = 0;
	f32 floatNum = 0;
	f32 multiplier = 0.1;
	union TokenData data;
	u32 start = *i;
	while (isdigit(source[*i])) {
		number = number * 10;
		number += (source[*i] - '0');
		*i += 1;
	}
	data.i = number;
	if (source[*i] == '.') {
		type = TOKEN_LITERAL_FLOAT;
		floatNum = (f32)number;
		*i += 1;
		while (isdigit(source[*i])) {
			floatNum += (f32)(source[*i] - '0') * multiplier;
			multiplier = multiplier * 0.1;
			*i += 1;
		}
		data.f = floatNum;
        log("Parsed Float: %f", data.f);
	} else {
        log("Parsed Int: %d", data.i);
    }
	PushTokenLiteral(tokens, type, StringAddS(table, substr(source, start, *i)),
					 lineNumber, data);
}

// Create a one-char SString
SString from_char(Allocator allocator, char c) {
	SString result;
	result.size = 1;
	result.data = Alloc(allocator, 2); // +1 for null-terminator
	result.data[0] = c;
	result.data[1] = '\0';
	log("Mem Alloc: %d", 2);
	return result;
}

static TokenType lookup_keyword(const SString* s) {
	if (SString_EqualsCString(s, "if"))
		return TOKEN_KEYWORD_IF;
	if (SString_EqualsCString(s, "else"))
		return TOKEN_KEYWORD_ELSE;
	if (SString_EqualsCString(s, "return"))
		return TOKEN_KEYWORD_RETURN;
	if (SString_EqualsCString(s, "let"))
		return TOKEN_KEYWORD_LET;
	if (SString_EqualsCString(s, "while"))
		return TOKEN_KEYWORD_WHILE;
	if (SString_EqualsCString(s, "for"))
		return TOKEN_KEYWORD_FOR;
	if (SString_EqualsCString(s, "int"))
		return TOKEN_KEYWORD_INT;
	if (SString_EqualsCString(s, "float"))
		return TOKEN_KEYWORD_FLOAT;
	if (SString_EqualsCString(s, "string"))
		return TOKEN_KEYWORD_STRING;
	if (SString_EqualsCString(s, "cell"))
		return TOKEN_KEYWORD_CELL;
	return TOKEN_INVALID;
}

static void handle_single_char_token(TokenList* tokens, StringTable* table,
									 const char* source, u32* i,
									 u32 lineNumber) {
	switch (source[*i]) {
	case '+':
		PushToken(tokens, TOKEN_CHAR_PLUS,
				  StringAddS(table, substr(source, *i, *i + 1)), lineNumber);
		break;
	case '-':
		PushToken(tokens, TOKEN_CHAR_MINUS,
				  StringAddS(table, substr(source, *i, *i + 1)), lineNumber);
		break;
	case '*':
		PushToken(tokens, TOKEN_CHAR_ASTERISK,
				  StringAddS(table, substr(source, *i, *i + 1)), lineNumber);
		break;
	case '/':
		PushToken(tokens, TOKEN_CHAR_SLASH,
				  StringAddS(table, substr(source, *i, *i + 1)), lineNumber);
		break;
	case '#':
		PushToken(tokens, TOKEN_CHAR_OCTOTHORPE,
				  StringAddS(table, substr(source, *i, *i + 1)), lineNumber);
		break;
	case ':':
		PushToken(tokens, TOKEN_CHAR_COLON,
				  StringAddS(table, substr(source, *i, *i + 1)), lineNumber);
		break;
	case '(':
		PushToken(tokens, TOKEN_CHAR_OPEN_PAREN,
				  StringAddS(table, substr(source, *i, *i + 1)), lineNumber);
		break;
	case ')':
		PushToken(tokens, TOKEN_CHAR_CLOSE_PAREN,
				  StringAddS(table, substr(source, *i, *i + 1)), lineNumber);
		break;
	case '[':
		PushToken(tokens, TOKEN_CHAR_OPEN_BRACKET,
				  StringAddS(table, substr(source, *i, *i + 1)), lineNumber);
		break;
	case ']':
		PushToken(tokens, TOKEN_CHAR_CLOSE_BRACKET,
				  StringAddS(table, substr(source, *i, *i + 1)), lineNumber);
		break;
	case '{':
		PushToken(tokens, TOKEN_CHAR_OPEN_BRACE,
				  StringAddS(table, substr(source, *i, *i + 1)), lineNumber);
		break;
	case '}':
		PushToken(tokens, TOKEN_CHAR_CLOSE_BRACE,
				  StringAddS(table, substr(source, *i, *i + 1)), lineNumber);
		break;
	case '=':
		if (source[*i + 1] == '=') {
			PushToken(tokens, TOKEN_DOUBLECHAR_EQUALS_EQUALS,
					  StringAddS(table, substr(source, *i, *i + 2)),
					  lineNumber);
			*i += 1;
		} else {
			PushToken(tokens, TOKEN_CHAR_EQUALS,
					  StringAddS(table, substr(source, *i, *i + 1)),
					  lineNumber);
		}
		break;
	case ',':
		PushToken(tokens, TOKEN_CHAR_COMMMA,
				  StringAddS(table, substr(source, *i, *i + 1)), lineNumber);
		break;
	case ';':
		PushToken(tokens, TOKEN_CHAR_SEMICOLON,
				  StringAddS(table, substr(source, *i, *i + 1)), lineNumber);
		break;
	case '>':
		if (source[*i + 1] == '=') {
			PushToken(tokens, TOKEN_DOUBLECHAR_GREATER_EQUALS,
					  StringAddS(table, substr(source, *i, *i + 2)),
					  lineNumber);
			*i += 1;
		} else {
			PushToken(tokens, TOKEN_CHAR_GREATER_THAN,
					  StringAddS(table, substr(source, *i, *i + 1)),
					  lineNumber);
		}
	case '<':
		if (source[*i + 1] == '=') {
			PushToken(tokens, TOKEN_DOUBLECHAR_LESS_EQUALS,
					  StringAddS(table, substr(source, *i, *i + 2)),
					  lineNumber);
			*i += 1;
		} else {
			PushToken(tokens, TOKEN_CHAR_LESS_THAN,
					  StringAddS(table, substr(source, *i, *i + 1)),
					  lineNumber);
		}
	case '!':
		if (source[*i + 1] == '=') {
			PushToken(tokens, TOKEN_DOUBLECHAR_EXCLAMATION_EQUALS,
					  StringAddS(table, substr(source, *i, *i + 2)),
					  lineNumber);
			*i += 1;
		} else {
			PushToken(tokens, TOKEN_CHAR_EXCLAMATION,
					  StringAddS(table, substr(source, *i, *i + 1)),
					  lineNumber);
		}
	case '&':
		if (source[*i + 1] == '&') {
			PushToken(tokens, TOKEN_DOUBLECHAR_AMPERSAND_AMPERSAND,
					  StringAddS(table, substr(source, *i, *i + 2)),
					  lineNumber);
			*i += 1;
		} else {
			log("Unknown char token: '%c'", source[*i]);
			PushToken(tokens, TOKEN_INVALID,
					  StringAddS(table, substr(source, *i, *i + 1)),
					  lineNumber);
		}
	case '|':
		if (source[*i + 1] == '|') {
			PushToken(tokens, TOKEN_DOUBLECHAR_PIPE_PIPE,
					  StringAddS(table, substr(source, *i, *i + 2)),
					  lineNumber);
			*i += 1;
		} else {
			log("Unknown char token: '%c'", source[*i]);
			PushToken(tokens, TOKEN_INVALID,
					  StringAddS(table, substr(source, *i, *i + 1)),
					  lineNumber);
		}
	default:
		log("Unknown char token: '%c'", source[*i]);
		PushToken(tokens, TOKEN_INVALID,
				  StringAddS(table, substr(source, *i, *i + 1)), lineNumber);
		break;
	}
}

TokenList* Tokenize(const char* source, StringTable* table,
					Allocator allocator) {
	SString source_s;
	source_s.data = (i8*)source;
	source_s.size = strlen(source);
	log("Beginning tokenization of input: %s", source_s);

	TokenList* tokens = CreateTokenList(allocator);
	u32 i = 0;
	u32 lineNumber = 0;

	while (source[i] != '\0') {
		char c = source[i];

		if (is_whitespace(c)) {
			if (c == '\n') {
				lineNumber++;
			}
			i++;
			continue;
		}

		if (is_alpha(c) || c == '_') {
			u32 start = i;
			while (is_alnum(source[i]) || source[i] == '_')
				i++;
			SString s = substr(source, start, i);
			TokenType type = lookup_keyword(&s);
			if (type == TOKEN_INVALID) {
				log("Parsed identifier: %s", s);
				PushToken(tokens, TOKEN_ID, StringAddS(table, s), lineNumber);
			} else {
				log("Parsed keyword: %s", s);
				PushToken(tokens, type, StringAddS(table, s), lineNumber);
			}
			continue;
		}

		if (isdigit(c)) {
			tokenizeNumberLiteral(source, &i, tokens, table, lineNumber);
			continue;
		}

		if (c == '\"') {
			tokenizeStringLiteral(source, allocator, &i, tokens, table,
								  &lineNumber);
			continue;
		}

		// Operators & symbols
		log("Parsed single-char token: '%c'\n", c);
		handle_single_char_token(tokens, table, source, &i, lineNumber);

		i++;
	}

	log("Finished tokenization.");
	return tokens;
}
