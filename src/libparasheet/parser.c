#include "util/util.h"
#include <libparasheet/lib_internal.h>
#include <libparasheet/tokenizer_types.h>

#define EPS UINT32_MAX
#define DEBUG 1

#define ExpectToken(tokens, expected, s)                                          \
	CheckToken(tokens, expected, syntaxError, s);                                 \
	if (*syntaxError) {                                                        \
		return EPS;                                                            \
	}

#define CheckNull(token, s)                                                       \
	if (token == NULL) {                                                       \
		UnconsumeToken(tokens);                                                \
		PrintNullTokenErrror(ConsumeToken(tokens), s);                         \
		*syntaxError = 1;                                                      \
		return EPS;                                                            \
	}

#define ThrowUnexpectedTokenError(unexpected, s)                                  \
	PrintUnexpectedTokenError(unexpected, s);                                     \
	*syntaxError = 1;                                                          \
	return EPS;

#define CheckSyntaxError()                                                     \
	if (*syntaxError) {                                                        \
		return EPS;                                                            \
	}

void PrintFoundDifferentError(TokenType expected, Token* found, StringTable* s) {
	// We probably want to have a better way to handle errors (so the user can
	// see errors in the ncurses TUI)
	warn("Syntax error on line %d: Expected %n but found \"%s\"",
		 found->lineNumber, getTokenErrorString(expected), StringGet(s, found->sourceString));
}

void PrintNullTokenErrror(Token* prev, StringTable* s) {
	warn("Syntax error on line %d: Expected token after \"%s\"",
		 prev->lineNumber, StringGet(s, prev->sourceString));
}

void PrintUnexpectedTokenError(Token* unexpected, StringTable* s) {
	// We probably want to have a better way to handle errors (so the user can
	// see errors in the ncurses TUI)
	warn("Syntax error on line %d: Unexpected token \"%s\"",
		 unexpected->lineNumber, StringGet(s, unexpected->sourceString));
}

void CheckToken(TokenList* tokens, TokenType expected, u8* syntaxError, StringTable* s) {
	Token* nextToken = ConsumeToken(tokens);
	if (nextToken == NULL) {
		UnconsumeToken(tokens);
		PrintNullTokenErrror(ConsumeToken(tokens), s);
		*syntaxError = 1;
		return;
	}
	if (nextToken->type != expected) {
		PrintFoundDifferentError(expected, nextToken, s);
		*syntaxError = 1;
	}
}
/*
void ConsumeUntilToken(TokenList* tokens, TokenType expected) {
	while (ConsumeToken(tokens)->type != expected)
		;
}
*/

// u32 is index into ast array
ASTNodeIndex ParseHeader(TokenList* tokens, AST* ast, u8* syntaxError, StringTable* s);
ASTNodeIndex ParseHeaderArgs(TokenList* tokens, AST* ast, u8* syntaxError, StringTable* s);
ASTNodeIndex ParseBlock(TokenList* tokens, AST* ast, u8* syntaxError, StringTable* s);
ASTNodeIndex ParseStatement(TokenList* tokens, AST* ast, u8* syntaxError, StringTable* s);
ASTNodeIndex ParseReturn(TokenList* tokens, AST* ast, u8* syntaxError, StringTable* s);
ASTNodeIndex ParseIf(TokenList* tokens, AST* ast, u8* syntaxError, StringTable* s);
ASTNodeIndex ParseWhile(TokenList* tokens, AST* ast, u8* syntaxError, StringTable* s);
ASTNodeIndex ParseFor(TokenList* tokens, AST* ast, u8* syntaxError, StringTable* s);
//ASTNodeIndex ParseExpression(TokenList* tokens, AST* ast, u8* syntaxError);
ASTNodeIndex ParseCellRef(TokenList* tokens, AST* ast, u8* syntaxError, StringTable* s);
ASTNodeIndex ParseID(TokenList* tokens, AST* ast, u8* syntaxError, StringTable* s);
ASTNodeIndex ParseLiteral(TokenList* tokens, AST* ast, u8* syntaxError, StringTable* s);
ASTNodeIndex ParseDatatype(TokenList* tokens, AST* ast, u8* syntaxError, StringTable* s);
ASTNodeIndex ParseUnit(TokenList* tokens, AST* ast, u8* syntaxError, StringTable* s);
ASTNodeIndex ParseDeclaration(TokenList* tokens, AST* ast, u8* syntaxError, StringTable* s);
ASTNodeIndex ParseFunctionCall(u32 cellRef, TokenList* tokens, AST* ast,
					  u8* syntaxError, StringTable* s);
ASTNodeIndex ParseFunctionArgs(TokenList* tokens, AST* ast, u8* syntaxError, StringTable* s);

ASTNodeIndex ParseExpression(TokenList* tokens, AST* ast, u8* syntaxError, StringTable* s);
ASTNodeIndex ParseExpression2(TokenList* tokens, AST* ast, u8* syntaxError, ASTNodeIndex inNode, StringTable* s);
ASTNodeIndex ParseSummation(TokenList* tokens, AST* ast, u8* syntaxError, StringTable* s);
ASTNodeIndex ParseSummation2(TokenList* tokens, AST* ast, u8* syntaxError, ASTNodeIndex inNode, StringTable* s);
ASTNodeIndex ParseTerm(TokenList* tokens, AST* ast, u8* syntaxError, StringTable* s);
ASTNodeIndex ParseTerm2(TokenList* tokens, AST* ast, u8* syntaxError, ASTNodeIndex inNode, StringTable* s);
ASTNodeIndex ParseReference(TokenList* tokens, AST* ast, u8* syntaxError, StringTable* s);
ASTNodeIndex ParseReference2(TokenList* tokens, AST* ast, u8* syntaxError, ASTNodeIndex inNode, StringTable* s);
ASTNodeIndex ParseAbsolute(TokenList* tokens, AST* ast, u8* syntaxError, StringTable* s);
ASTNodeIndex ParseAbsolute2(TokenList* tokens, AST* ast, u8* syntaxError, ASTNodeIndex inNode, StringTable* s);


AST BuildASTFromTokens(TokenList* tokens, StringTable* s, Allocator allocator) {
	AST ast = {
        .mem = allocator
    };

	u8 syntaxError = 0;
	(void)!ParseHeader(tokens, &ast, &syntaxError, s);

	if (syntaxError) {
		ASTFree(&ast);

		return (AST){0};
	}
	
	return ast;
}

ASTNodeIndex ParseHeader(TokenList* tokens, AST* ast, u8* syntaxError, StringTable* s) {
	ExpectToken(tokens, TOKEN_CHAR_EQUALS, s);
	Token* nextToken = ConsumeToken(tokens);
	CheckNull(nextToken, s);
	if (nextToken->type == TOKEN_CHAR_OPEN_PAREN) {
		ASTNodeIndex headerArgs = ParseHeaderArgs(tokens, ast, syntaxError, s);
		CheckSyntaxError();
		ASTNodeIndex mainBlock = ParseBlock(tokens, ast, syntaxError, s);
		CheckSyntaxError();

		return ASTCreateNode(ast, AST_HEADER, headerArgs, mainBlock, EPS);
	} else {
        UnconsumeToken(tokens);
		return ParseBlock(tokens, ast, syntaxError, s);
	}
}

// This doesn't expect an open parenthesis token because the caller already
// consumed it before calling
ASTNodeIndex ParseHeaderArgs(TokenList* tokens, AST* ast, u8* syntaxError, StringTable* s) {
	Token* nextToken = ConsumeToken(tokens);
	CheckNull(nextToken, s);
	if (nextToken->type == TOKEN_CHAR_CLOSE_PAREN) {
		return EPS;
	}
	UnconsumeToken(tokens);
	ASTNodeIndex identifier = ParseID(tokens, ast, syntaxError, s);
	CheckSyntaxError();
	ExpectToken(tokens, TOKEN_CHAR_COLON, s);
	ASTNodeIndex type = ParseDatatype(tokens, ast, syntaxError, s);
	CheckSyntaxError();
	nextToken = ConsumeToken(tokens);
	CheckNull(nextToken, s);
	ASTNodeIndex nextArgs;
	if (nextToken->type == TOKEN_CHAR_COMMMA) {
		nextArgs = ParseHeaderArgs(tokens, ast, syntaxError, s);
		CheckSyntaxError();
	} else if (nextToken->type == TOKEN_CHAR_CLOSE_PAREN) {
		nextArgs = EPS;
	} else {
		ThrowUnexpectedTokenError(nextToken, s);
	}

	return ASTCreateNode(ast, AST_HEADER_ARGS, identifier, type, nextArgs);
}

ASTNodeIndex ParseBlock(TokenList* tokens, AST* ast, u8* syntaxError, StringTable* s) {
	ASTNodeIndex statement = ParseStatement(tokens, ast, syntaxError, s);
	CheckSyntaxError();
	if (statement == EPS) {
		return EPS;
	}

	ASTNodeIndex continueBlock = ParseBlock(tokens, ast, syntaxError, s);
	CheckSyntaxError();
	if (continueBlock == EPS) {
        u32 scope_end = ASTCreateNode(ast, AST_SCOPE_END, EPS, EPS, EPS);
        return ASTCreateNode(ast, AST_SEQ, statement, scope_end, EPS);
	}

	return ASTCreateNode(ast, AST_SEQ, statement, continueBlock, EPS);
}

ASTNodeIndex ParseStatement(TokenList* tokens, AST* ast, u8* syntaxError, StringTable* s) {
	Token* nextToken = ConsumeToken(tokens);
	if (nextToken == NULL) {
		return EPS;
	}
    log("next: %s", getTokenErrorString(nextToken->type));
	ASTNodeIndex tmp;
	switch (nextToken->type) {
	case TOKEN_CHAR_SEMICOLON:
		return ParseStatement(tokens, ast, syntaxError, s);
	case TOKEN_CHAR_CLOSE_BRACE:
		return EPS;
	case TOKEN_KEYWORD_IF:
		return ParseIf(tokens, ast, syntaxError, s);
	case TOKEN_KEYWORD_WHILE:
		return ParseWhile(tokens, ast, syntaxError, s);
	case TOKEN_KEYWORD_FOR:
		return ParseFor(tokens, ast, syntaxError, s);
	case TOKEN_KEYWORD_RETURN:
		tmp = ParseReturn(tokens, ast, syntaxError, s);
		CheckSyntaxError();
		ExpectToken(tokens, TOKEN_CHAR_SEMICOLON, s);
		return tmp;
		break;
	case TOKEN_CHAR_OPEN_BRACE:
		tmp = ParseBlock(tokens, ast, syntaxError, s);
		CheckSyntaxError();
		ExpectToken(tokens, TOKEN_CHAR_CLOSE_BRACE, s);
        u32 scope_start = ASTCreateNode(ast, AST_SCOPE_BEGIN, EPS, EPS, EPS);
		return ASTCreateNode(ast, AST_SEQ, scope_start, tmp, EPS);
		break;
	default:
		UnconsumeToken(tokens);
		ASTNodeIndex expression = ParseExpression(tokens, ast, syntaxError, s);
		CheckSyntaxError();
		ExpectToken(tokens, TOKEN_CHAR_SEMICOLON, s);
		return expression;
		break;
	}
}

// This doesn't expect a return token because the caller already consumed it
// before calling
ASTNodeIndex ParseReturn(TokenList* tokens, AST* ast, u8* syntaxError, StringTable* s) {
	ASTNodeIndex returnValue = ParseExpression(tokens, ast, syntaxError, s);
	CheckSyntaxError();
	return ASTCreateNode(ast, AST_RETURN, returnValue, EPS, EPS);
}

// This doesn't expect an if token because the caller already consumed it before
// calling
ASTNodeIndex ParseIf(TokenList* tokens, AST* ast, u8* syntaxError, StringTable* s) {
	ExpectToken(tokens, TOKEN_CHAR_OPEN_PAREN, s);
	ASTNodeIndex condition = ParseExpression(tokens, ast, syntaxError, s);
	CheckSyntaxError();
	ExpectToken(tokens, TOKEN_CHAR_CLOSE_PAREN, s);
	ASTNodeIndex body = ParseStatement(tokens, ast, syntaxError, s);
	CheckSyntaxError();

	Token* nextToken = ConsumeToken(tokens);
	CheckNull(nextToken, s);
	ASTNodeIndex elseBody;
	if (nextToken != NULL) {
		if (nextToken->type == TOKEN_KEYWORD_ELSE) {
			elseBody = ParseStatement(tokens, ast, syntaxError, s);
			CheckSyntaxError();
		} else {
			elseBody = EPS;
			UnconsumeToken(tokens);
		}
	} else {
		elseBody = EPS;
	}

	return ASTCreateNode(ast, AST_IF_ELSE, condition, body, elseBody);
}

// This doesn't expect a while token because the caller already consumed it
// before calling
ASTNodeIndex ParseWhile(TokenList* tokens, AST* ast, u8* syntaxError, StringTable* s) {
	ExpectToken(tokens, TOKEN_CHAR_OPEN_PAREN, s);
	ASTNodeIndex condition = ParseExpression(tokens, ast, syntaxError, s);
	CheckSyntaxError();
	ExpectToken(tokens, TOKEN_CHAR_CLOSE_PAREN, s);
	ASTNodeIndex body = ParseStatement(tokens, ast, syntaxError, s);
	CheckSyntaxError();

	return ASTCreateNode(ast, AST_WHILE, condition, body, EPS);
}

// This doesn't expect a for token because the caller already consumed it before
// calling
ASTNodeIndex ParseFor(TokenList* tokens, AST* ast, u8* syntaxError, StringTable* s) {
	ExpectToken(tokens, TOKEN_CHAR_OPEN_PAREN, s);
	ASTNodeIndex iterator = ParseID(tokens, ast, syntaxError, s);
	CheckSyntaxError();
	ExpectToken(tokens, TOKEN_CHAR_COMMMA, s);
	ASTNodeIndex range = ParseExpression(tokens, ast, syntaxError, s);
	CheckSyntaxError();
	ExpectToken(tokens, TOKEN_CHAR_CLOSE_PAREN, s);
	ASTNodeIndex body = ParseStatement(tokens, ast, syntaxError, s);
	CheckSyntaxError();

	return ASTCreateNode(ast, AST_FOR, iterator, range, body);
}

// Expression parsing. A little scuffed due to a change in primitives. We'll survive.
ASTNodeIndex ParseExpression(TokenList* tokens, AST* ast, u8* syntaxError, StringTable* s) {
    Token* peek = PeekToken(tokens);
    if (peek->type == TOKEN_LITERAL_INT || peek->type == TOKEN_LITERAL_FLOAT ||
			peek->type == TOKEN_LITERAL_STRING || peek->type == TOKEN_CHAR_OPEN_PAREN ||
        peek->type == TOKEN_ID || peek->type == TOKEN_KEYWORD_LET){
        ASTNodeIndex node = ParseSummation(tokens, ast, syntaxError, s);
        ASTNodeIndex node2 = ParseExpression2(tokens, ast, syntaxError, node, s);
		return node2;
    }
	return EPS;
}
ASTNodeIndex ParseExpression2(TokenList* tokens, AST* ast, u8* syntaxError,
		ASTNodeIndex inNode, StringTable* s) {
    Token* peek = PeekToken(tokens);
    if (peek->type == TOKEN_CHAR_EQUALS){
        ExpectToken(tokens, TOKEN_CHAR_EQUALS, s);
        ASTNodeIndex node = ParseSummation(tokens, ast, syntaxError, s);
		ASTNodeIndex curr = ASTCreateNode(ast, AST_ASSIGN_VALUE, inNode, node, EPS);
        ASTNodeIndex node2 = ParseExpression2(tokens, ast, syntaxError, curr, s);
        return node2;
    } else if (peek->type == TOKEN_CHAR_SEMICOLON || peek->type == TOKEN_CHAR_CLOSE_PAREN){
        return inNode;
    } else {
		return EPS;
	}
}
ASTNodeIndex ParseSummation(TokenList* tokens, AST* ast, u8* syntaxError, StringTable* s) {
    Token* peek = PeekToken(tokens);
    if (peek->type == TOKEN_LITERAL_INT || peek->type == TOKEN_LITERAL_FLOAT ||
        peek->type == TOKEN_LITERAL_STRING || peek->type == TOKEN_CHAR_OPEN_PAREN || peek->type == TOKEN_ID || peek->type == TOKEN_KEYWORD_LET){
		ASTNodeIndex node = ParseTerm(tokens, ast, syntaxError, s);
		ASTNodeIndex node2 = ParseSummation2(tokens, ast, syntaxError, node, s);
		return node2;
    }
	return EPS;
}
ASTNodeIndex ParseSummation2(TokenList* tokens, AST* ast, u8* syntaxError, ASTNodeIndex inNode, StringTable* s) {
    Token* peek = PeekToken(tokens);
    if (peek->type == TOKEN_CHAR_PLUS){
        ExpectToken(tokens, TOKEN_CHAR_PLUS, s);
        ASTNodeIndex node = ParseTerm(tokens, ast, syntaxError, s);
		ASTNodeIndex curr = ASTCreateNode(ast, AST_ADD, inNode, node, EPS);
        ASTNodeIndex node2 = ParseSummation2(tokens, ast, syntaxError, curr, s);
        return node2;
    }
    if (peek->type == TOKEN_CHAR_MINUS){
        ExpectToken(tokens, TOKEN_CHAR_MINUS, s);
        ASTNodeIndex node = ParseTerm(tokens, ast, syntaxError, s);
		ASTNodeIndex curr = ASTCreateNode(ast, AST_SUB, inNode, node, EPS);
        ASTNodeIndex node2 = ParseSummation2(tokens, ast, syntaxError, curr, s);
        return node2;
    }
    if (peek->type == TOKEN_CHAR_SEMICOLON || peek->type == TOKEN_CHAR_CLOSE_PAREN || peek->type == TOKEN_CHAR_EQUALS){
        return inNode;
    }
}
ASTNodeIndex ParseTerm(TokenList* tokens, AST* ast, u8* syntaxError, StringTable* s) {
    Token* peek = PeekToken(tokens);
    if (peek->type == TOKEN_LITERAL_INT || peek->type == TOKEN_LITERAL_FLOAT ||
        peek->type == TOKEN_LITERAL_STRING || peek->type == TOKEN_CHAR_OPEN_PAREN || peek->type == TOKEN_ID || peek->type == TOKEN_KEYWORD_LET){
        ASTNodeIndex node = ParseReference(tokens, ast, syntaxError, s);
        ASTNodeIndex node2 = ParseTerm2(tokens, ast, syntaxError, node, s);
        return node2;
    }
	return EPS;
}
ASTNodeIndex ParseTerm2(TokenList* tokens, AST* ast, u8* syntaxError, ASTNodeIndex inNode, StringTable* s) {
    Token* peek = PeekToken(tokens);
    if (peek->type == TOKEN_CHAR_ASTERISK){
        ExpectToken(tokens, TOKEN_CHAR_ASTERISK, s);
        ASTNodeIndex node = ParseReference(tokens, ast, syntaxError, s);
		ASTNodeIndex curr = ASTCreateNode(ast, AST_MUL, inNode, node, EPS);
        ASTNodeIndex node2 = ParseTerm2(tokens, ast, syntaxError, curr, s);
        return node2;
    }
    if (peek->type == TOKEN_CHAR_SLASH){
        ExpectToken(tokens, TOKEN_CHAR_SLASH, s);
        ASTNodeIndex node = ParseReference(tokens, ast, syntaxError, s);
		ASTNodeIndex curr = ASTCreateNode(ast, AST_DIV, inNode, node, EPS);
        ASTNodeIndex node2 = ParseTerm2(tokens, ast, syntaxError, curr, s);
        return node2;
    }
    if (peek->type == TOKEN_CHAR_SEMICOLON || peek->type == TOKEN_CHAR_CLOSE_PAREN || peek->type == TOKEN_CHAR_EQUALS || peek->type == TOKEN_CHAR_PLUS || peek->type == TOKEN_CHAR_MINUS){
        return inNode;
    }
	return EPS;
}
ASTNodeIndex ParseReference(TokenList* tokens, AST* ast, u8* syntaxError, StringTable* s) {
    Token* peek = PeekToken(tokens);
    if (peek->type == TOKEN_LITERAL_INT || peek->type == TOKEN_LITERAL_FLOAT ||
        peek->type == TOKEN_LITERAL_STRING || peek->type == TOKEN_CHAR_OPEN_PAREN || peek->type == TOKEN_ID || peek->type == TOKEN_KEYWORD_LET){
        ASTNodeIndex node = ParseAbsolute(tokens, ast, syntaxError, s);
        ASTNodeIndex node2 = ParseReference2(tokens, ast, syntaxError, node, s);
        return node2;
    }
	return EPS;
}
ASTNodeIndex ParseReference2(TokenList* tokens, AST* ast, u8* syntaxError, ASTNodeIndex inNode, StringTable* s) {
    Token* peek = PeekToken(tokens);
    if (peek->type == TOKEN_CHAR_COLON){
        ExpectToken(tokens, TOKEN_CHAR_COLON, s);
        ASTNodeIndex node = ParseAbsolute(tokens, ast, syntaxError, s);
		ASTNodeIndex curr = ASTCreateNode(ast, AST_RANGE, inNode, node, EPS);
        ASTNodeIndex node2 = ParseReference2(tokens, ast, syntaxError, curr, s);
        return node2;
    }
    if (peek->type == TOKEN_CHAR_SEMICOLON || peek->type == TOKEN_CHAR_CLOSE_PAREN || peek->type == TOKEN_CHAR_EQUALS || peek->type == TOKEN_CHAR_PLUS || peek->type == TOKEN_CHAR_MINUS || peek->type == TOKEN_CHAR_ASTERISK || peek->type == TOKEN_CHAR_SLASH){
        return inNode;
    }
	return EPS;
}

ASTNodeIndex ParseAbsolute(TokenList* tokens, AST* ast, u8* syntaxError, StringTable* s) {
    Token* peek = PeekToken(tokens);
    if (peek->type == TOKEN_LITERAL_INT || peek->type == TOKEN_LITERAL_FLOAT ||
        peek->type == TOKEN_LITERAL_STRING || peek->type == TOKEN_CHAR_OPEN_PAREN || peek->type == TOKEN_ID || peek->type == TOKEN_KEYWORD_LET){
        ASTNodeIndex node = ParseUnit(tokens, ast, syntaxError, s);
        return node;
    }
    if (peek->type == TOKEN_CHAR_OCTOTHORPE){
        ExpectToken(tokens, TOKEN_CHAR_OCTOTHORPE, s);
        ASTNodeIndex node = ParseUnit(tokens, ast, syntaxError, s);
        return ASTCreateNode(ast, AST_COORD_TRANSFORM, node, EPS, EPS);
    }
	return EPS;
}

// This doesn't expect an open bracket token because the caller already consumed
// it before calling
ASTNodeIndex ParseCellRef(TokenList* tokens, AST* ast, u8* syntaxError, StringTable* s) {
	ASTNodeIndex x = ParseExpression(tokens, ast, syntaxError, s);
	CheckSyntaxError();
	ExpectToken(tokens, TOKEN_CHAR_COMMMA, s);
	ASTNodeIndex y = ParseExpression(tokens, ast, syntaxError, s);
	CheckSyntaxError();
	ExpectToken(tokens, TOKEN_CHAR_CLOSE_BRACKET, s);

	return ASTCreateNode(ast, AST_GET_CELL_REF, x, y, EPS);
}

ASTNodeIndex ParseID(TokenList* tokens, AST* ast, u8* syntaxError, StringTable* s) {
	Token* id = ConsumeToken(tokens);
	CheckNull(id, s);

	ASTNodeIndex new_node_index = ASTCreateNode(ast, AST_ID, EPS, EPS, EPS);

	ast->nodes[new_node_index].vt = V_INT;
	ast->nodes[new_node_index].data.s = id->sourceString;

	return new_node_index;
}

// TODO: Actually get literal values from token string (this should probably be
// a function in the tokenizer that is called here)
// clarise TODO: replace literal values with literal->data once rebased with tokenizer fixes
ASTNodeIndex ParseLiteral(TokenList* tokens, AST* ast, u8* syntaxError, StringTable* s) {
	Token* literal = ConsumeToken(tokens);
	CheckNull(literal, s);

	ASTNodeIndex new_node_index = ASTCreateNode(ast, AST_INVALID, EPS, EPS, EPS);

    switch (literal->type) {
        case (TOKEN_LITERAL_INT):
            ast->nodes[new_node_index].op = AST_INT_LITERAL;
            ast->nodes[new_node_index].vt = V_INT;
            ast->nodes[new_node_index].data.i = literal->data.i; // TODO: Get literal values
            break;
        case (TOKEN_LITERAL_FLOAT):
            ast->nodes[new_node_index].op = AST_FLOAT_LITERAL;
            ast->nodes[new_node_index].vt = V_FLOAT;
            ast->nodes[new_node_index].data.f = literal->data.f; // TODO: Get literal values
            break;
        case (TOKEN_LITERAL_STRING):
            ast->nodes[new_node_index].op = AST_INVALID; // TODO: Add strings
            ast->nodes[new_node_index].vt = V_INT;		 // Interned string
            //ast->nodes[new_node_index].data.i = literal->data.s;		 // TODO: Get literal values
            break;
        default:
            break;
    }

	return new_node_index;
}

ASTNodeIndex ParseDatatype(TokenList* tokens, AST* ast, u8* syntaxError, StringTable* s) {
	Token* dataType = ConsumeToken(tokens);
	CheckNull(dataType, s);

	ASTNodeIndex new_node_index = ASTCreateNode(ast, AST_INVALID, EPS, EPS, EPS);

	switch (dataType->type) {
	case (TOKEN_KEYWORD_INT):
		ast->nodes[new_node_index].op = AST_INT_TYPE;
		break;
	case (TOKEN_KEYWORD_FLOAT):
		ast->nodes[new_node_index].op = AST_FLOAT_TYPE;
		break;
	case (TOKEN_KEYWORD_STRING):
		ast->nodes[new_node_index].op = AST_INVALID; // TODO: Add strings
		break;
	default:
		break;
	}

	return new_node_index;
}

ASTNodeIndex ParseUnit(TokenList* tokens, AST* ast, u8* syntaxError, StringTable* s) {
	Token* unit = ConsumeToken(tokens);
	CheckNull(unit, s);

	ASTNodeIndex tmp;
	switch (unit->type) {
    case TOKEN_KEYWORD_LET:
        return ParseDeclaration(tokens, ast, syntaxError, s); 
	case TOKEN_LITERAL_INT:
		UnconsumeToken(tokens);
		return ParseLiteral(tokens, ast, syntaxError, s);
	case TOKEN_LITERAL_FLOAT:
		UnconsumeToken(tokens);
		return ParseLiteral(tokens, ast, syntaxError, s);
	case TOKEN_LITERAL_STRING:
		UnconsumeToken(tokens);
		return ParseLiteral(tokens, ast, syntaxError, s);
	case TOKEN_CHAR_OPEN_PAREN:
        //ConsumeToken(tokens);
		tmp = ParseExpression(tokens, ast, syntaxError, s);
		CheckSyntaxError();
		ExpectToken(tokens, TOKEN_CHAR_CLOSE_PAREN, s);
		return tmp;
	case TOKEN_ID:
		UnconsumeToken(tokens);
		return ParseID(tokens, ast, syntaxError, s);
	default:
		ThrowUnexpectedTokenError(unit, s);
		return EPS;
	}
}

// The function should have already been consumed because of how expression
// parsing currently works
ASTNodeIndex ParseFunctionCall(ASTNodeIndex cellRef, TokenList* tokens, AST* ast,
					  u8* syntaxError, StringTable* s) {
	ASTNodeIndex args = ParseFunctionArgs(tokens, ast, syntaxError, s);
	CheckSyntaxError();
	return ASTCreateNode(ast, AST_CALL, cellRef, args, EPS);
}

// This doesn't expect an open parenthesis token because the caller already
// consumed it before calling
ASTNodeIndex ParseFunctionArgs(TokenList* tokens, AST* ast, u8* syntaxError, StringTable* s) {
	Token* nextToken = ConsumeToken(tokens);
	CheckNull(nextToken, s);
	if (nextToken->type == TOKEN_CHAR_CLOSE_PAREN) {
		return EPS;
	}
	UnconsumeToken(tokens);
	ASTNodeIndex arg = ParseID(tokens, ast, syntaxError, s);
	CheckSyntaxError();
	nextToken = ConsumeToken(tokens);
	CheckNull(nextToken, s);
	ASTNodeIndex nextArgs;
	if (nextToken->type == TOKEN_CHAR_COMMMA) {
		nextArgs = ParseFunctionArgs(tokens, ast, syntaxError, s);
		CheckSyntaxError();
	} else if (nextToken->type == TOKEN_CHAR_CLOSE_PAREN) {
		nextArgs = EPS;
	} else {
		ThrowUnexpectedTokenError(nextToken, s);
		return EPS;
	}
	return ASTCreateNode(ast, AST_FUNC_ARGS, arg, nextArgs, EPS);
}

ASTNodeIndex ParseDeclaration(TokenList* tokens, AST* ast, u8* syntaxError, StringTable* s) {

    Token* id = ConsumeToken(tokens);
    CheckNull(id, s);
    ExpectToken(tokens, TOKEN_CHAR_COLON, s);

    Token* type = ConsumeToken(tokens);
    CheckNull(type, s);

    u32 n = ASTCreateNode(ast, AST_DECLARE_VARIABLE, EPS, EPS, EPS);

    switch (type->type) {
        case TOKEN_KEYWORD_INT:
            ASTGet(ast, n).vt = V_INT;
        break;
        case TOKEN_KEYWORD_FLOAT:
            ASTGet(ast, n).vt = V_FLOAT;
        break;

        default:
            ThrowUnexpectedTokenError(type, s);
    }

    ASTGet(ast, n).data.s = id->sourceString;
    log("Parsed Declaration: %s", StringGet(s, ASTGet(ast, n).data.s));

    return n;
}
