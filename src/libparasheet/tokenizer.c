#include <libparasheet/tokenizer_types.h>
#include <util/util.h>
#include <string.h>

static bool is_whitespace(char c) {
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static bool is_alpha(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

// static bool is_digit(char c) {
// 	return c >= '0' && c <= '9';
// }

static bool is_alnum(char c) {
	return is_alpha(c) || isdigit(c);
}

// Internal string compare helper
static bool SString_EqualsCString(const SString* s, const char* cstr) {
	u32 len = 0;
	while (cstr[len]) len++;
	if (s->size != len) return false;
	return memcmp(s->data, cstr, len) == 0;
}

// Create a new string from source[start:end]
SString substr(Allocator allocator, const char* src, u32 start, u32 end) {
	u32 len = end - start;
	SString result;
	result.size = len;
	result.data = Alloc(allocator, len + 1); // +1 for '\0'
	memcpy(result.data, src + start, len);
	result.data[len] = '\0'; // null terminate
	log("Mem Alloc: %d", len + 1);
	return result;
}

// Create a one-char SString
SString from_char(Allocator allocator, char c) {
	SString result;
	result.size = 1;
	result.data = Alloc(allocator, 2);  // +1 for null-terminator
	result.data[0] = c;
	result.data[1] = '\0';
	log("Mem Alloc: %d", 2);
	return result;
}

static u32 fake_symbol_table_insert(SString id) {
	return 0; // Placeholder
}

static TokenType lookup_keyword(const SString* s) {
	if (SString_EqualsCString(s, "if"))     return TOKEN_KEYWORD_IF;
	if (SString_EqualsCString(s, "else"))   return TOKEN_KEYWORD_ELSE;
	if (SString_EqualsCString(s, "return")) return TOKEN_KEYWORD_RETURN;
	if (SString_EqualsCString(s, "let"))    return TOKEN_KEYWORD_LET;
	if (SString_EqualsCString(s, "while"))  return TOKEN_KEYWORD_WHILE;
	if (SString_EqualsCString(s, "for"))    return TOKEN_KEYWORD_FOR;
	if (SString_EqualsCString(s, "int"))    return TOKEN_KEYWORD_INT;
	if (SString_EqualsCString(s, "float"))  return TOKEN_KEYWORD_FLOAT;
	if (SString_EqualsCString(s, "string")) return TOKEN_KEYWORD_STRING;
	if (SString_EqualsCString(s, "cell"))   return TOKEN_KEYWORD_CELL;
	return TOKEN_INVALID;
}

static void handle_single_char_token(TokenList* tokens, Allocator allocator, char c) {
	switch (c) {
		case '+': PushToken(tokens, TOKEN_OP_PLUS, from_char(allocator, c)); break;
		case '-': PushToken(tokens, TOKEN_OP_MINUS, from_char(allocator, c)); break;
		case '*': PushToken(tokens, TOKEN_OP_TIMES, from_char(allocator, c)); break;
		case '/': PushToken(tokens, TOKEN_OP_DIVIDE, from_char(allocator, c)); break;
		case '#': PushToken(tokens, TOKEN_OP_OCTOTHORPE, from_char(allocator, c)); break;
		case ':': PushToken(tokens, TOKEN_OP_COLON, from_char(allocator, c)); break;
		case '(': PushToken(tokens, TOKEN_GROUPING_OPEN_PAREN, from_char(allocator, c)); break;
		case ')': PushToken(tokens, TOKEN_GROUPING_CLOSE_PAREN, from_char(allocator, c)); break;
		case '[': PushToken(tokens, TOKEN_GROUPING_OPEN_BRACKET, from_char(allocator, c)); break;
		case ']': PushToken(tokens, TOKEN_GROUPING_CLOSE_BRACKET, from_char(allocator, c)); break;
		case '{': PushToken(tokens, TOKEN_GROUPING_OPEN_BRACE, from_char(allocator, c)); break;
		case '}': PushToken(tokens, TOKEN_GROUPING_CLOSE_BRACE, from_char(allocator, c)); break;
		default:
			log("Unknown char token: '%c'", c);
			PushToken(tokens, TOKEN_INVALID, from_char(allocator, c));
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

	while (source[i] != '\0') {
		char c = source[i];

		if (is_whitespace(c)) {
			i++;
			continue;
		}

		if (is_alpha(c) || c == '_') {
			u32 start = i;
			while (is_alnum(source[i]) || source[i] == '_') i++;
			SString s = substr(allocator, source, start, i);
			TokenType type = lookup_keyword(&s);
			if (type == TOKEN_INVALID) {
				u32 idx = fake_symbol_table_insert(s);
				log("Parsed identifier: %s", s);
				PushTokenID(tokens, TOKEN_ID, s, idx);
			} else {
				log("Parsed keyword: %s", s);
				PushToken(tokens, type, s);
			}
			continue;
		}

		if (isdigit(c)) {
			u32 start = i;
			while (isdigit(source[i])) i++;
			SString s = substr(allocator, source, start, i);
			log("Parsed integer literal: %s", s);
			PushToken(tokens, TOKEN_LITERAL_INT, s);
			continue;
		}

		if (c == '"') {
			i++;
			u32 start = i;
			while (source[i] && source[i] != '"') i++;
			SString s = substr(allocator, source, start, i);
			if (source[i] == '"') i++;
			log("Parsed string literal: %s", s);
			PushToken(tokens, TOKEN_LITERAL_STRING, s);
			continue;
		}

		// Operators & symbols
        fprintf(stderr, "Parsed single-char token: '%c'\n", c);
        handle_single_char_token(tokens, allocator, c);

		i++;
	}

	log("Finished tokenization.");
	return tokens;
}
