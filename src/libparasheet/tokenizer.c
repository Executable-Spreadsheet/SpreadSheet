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

static u32 fake_symbol_table_insert(SString id) {
	return 0; // Placeholder
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

static void handle_single_char_token(TokenList* tokens, Allocator allocator,
									 char c, u32 lineNumber) {
	switch (c) {
	case '+':
		PushToken(tokens, TOKEN_CHAR_PLUS, from_char(allocator, c), lineNumber);
		break;
	case '-':
		PushToken(tokens, TOKEN_CHAR_MINUS, from_char(allocator, c),
				  lineNumber);
		break;
	case '*':
		PushToken(tokens, TOKEN_CHAR_ASTERISK, from_char(allocator, c),
				  lineNumber);
		break;
	case '/':
		PushToken(tokens, TOKEN_CHAR_SLASH, from_char(allocator, c),
				  lineNumber);
		break;
	case '#':
		PushToken(tokens, TOKEN_CHAR_OCTOTHORPE, from_char(allocator, c),
				  lineNumber);
		break;
	case ':':
		PushToken(tokens, TOKEN_CHAR_COLON, from_char(allocator, c),
				  lineNumber);
		break;
	case '(':
		PushToken(tokens, TOKEN_CHAR_OPEN_PAREN, from_char(allocator, c),
				  lineNumber);
		break;
	case ')':
		PushToken(tokens, TOKEN_CHAR_CLOSE_PAREN, from_char(allocator, c),
				  lineNumber);
		break;
	case '[':
		PushToken(tokens, TOKEN_CHAR_OPEN_BRACKET, from_char(allocator, c),
				  lineNumber);
		break;
	case ']':
		PushToken(tokens, TOKEN_CHAR_CLOSE_BRACKET, from_char(allocator, c),
				  lineNumber);
		break;
	case '{':
		PushToken(tokens, TOKEN_CHAR_OPEN_BRACE, from_char(allocator, c),
				  lineNumber);
		break;
	case '}':
		PushToken(tokens, TOKEN_CHAR_CLOSE_BRACE, from_char(allocator, c),
				  lineNumber);
		break;
	case '=':
		PushToken(tokens, TOKEN_CHAR_EQUALS, from_char(allocator, c),
				  lineNumber);
		break;
	case ',':
		PushToken(tokens, TOKEN_CHAR_COMMMA, from_char(allocator, c),
				  lineNumber);
		break;
	case ';':
		PushToken(tokens, TOKEN_CHAR_SEMICOLON, from_char(allocator, c),
				  lineNumber);
		break;
	default:
		log("Unknown char token: '%c'", c);
		PushToken(tokens, TOKEN_INVALID, from_char(allocator, c), lineNumber);
		break;
	}
}

TokenList* Tokenize(const char* source, Allocator allocator) {
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
				u32 idx = fake_symbol_table_insert(s);
				log("Parsed identifier: %s", s);
				PushTokenID(tokens, TOKEN_ID, s, idx, lineNumber);
			} else {
				log("Parsed keyword: %s", s);
				PushToken(tokens, type, s, lineNumber);
			}
			continue;
		}

		if (isdigit(c)) {
			u32 start = i;
			while (isdigit(source[i]))
				i++;
			SString s = substr(source, start, i);
			log("Parsed integer literal: %s", s);
			PushToken(tokens, TOKEN_LITERAL_INT, s, lineNumber);
			continue;
		}

		if (c == '"') {
			i++;
			u32 start = i;
			while (source[i] && source[i] != '"')
				i++;
			SString s = substr(source, start, i);
			if (source[i] == '"')
				i++;
			log("Parsed string literal: %s", s);
			PushToken(tokens, TOKEN_LITERAL_STRING, s, lineNumber);
			continue;
		}

		// Operators & symbols
		log("Parsed single-char token: '%c'\n", c);
		handle_single_char_token(tokens, allocator, c, lineNumber);

		i++;
	}

	log("Finished tokenization.");
	return tokens;
}
