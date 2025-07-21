#include "util/util.h"
#include <libparasheet/lib_internal.h>
#include <libparasheet/tokenizer_types.h>

#define EPS UINT32_MAX
#define DEBUG 1

#define ExpectToken(tokens, expected)                                          \
	CheckToken(tokens, expected, syntaxError);                                 \
	if (*syntaxError) {                                                        \
		return EPS;                                                            \
	}

#define CheckNull(token)                                                       \
	if (token == NULL) {                                                       \
		UnconsumeToken(tokens);                                                \
		PrintNullTokenErrror(ConsumeToken(tokens));                            \
		*syntaxError = 1;                                                      \
		return EPS;                                                            \
	}

#define ThrowUnexpectedTokenError(unexpected)                                  \
	PrintUnexpectedTokenError(unexpected);                                     \
	*syntaxError = 1;                                                          \
	return EPS;

#define CheckSyntaxError()                                                     \
	if (*syntaxError) {                                                        \
		return EPS;                                                            \
	}

void PrintFoundDifferentError(TokenType expected, Token* found) {
	// We probably want to have a better way to handle errors (so the user can
	// see errors in the ncurses TUI)
	warn("Syntax error on line %d: Expected %s but found \"%s\"",
		 found->lineNumber, getTokenErrorString(expected), found->sourceString);
}

void PrintNullTokenErrror(Token* prev) {
	warn("Syntax error on line %d: Expected token after \"%s\"",
		 prev->lineNumber, prev->sourceString);
}

void PrintUnexpectedTokenError(Token* unexpected) {
	// We probably want to have a better way to handle errors (so the user can
	// see errors in the ncurses TUI)
	warn("Syntax error on line %d: Unexpected token \"%s\"",
		 unexpected->lineNumber, unexpected->sourceString);
}

void CheckToken(TokenList* tokens, TokenType expected, u8* syntaxError) {
	Token* nextToken = ConsumeToken(tokens);
	if (nextToken == NULL) {
		UnconsumeToken(tokens);
		PrintNullTokenErrror(ConsumeToken(tokens));
		*syntaxError = 1;
		return;
	}
	if (nextToken->type != expected) {
		PrintFoundDifferentError(expected, nextToken);
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
ASTNodeIndex ParseHeader(TokenList* tokens, AST* ast, u8* syntaxError);
ASTNodeIndex ParseHeaderArgs(TokenList* tokens, AST* ast, u8* syntaxError);
ASTNodeIndex ParseBlock(TokenList* tokens, AST* ast, u8* syntaxError);
ASTNodeIndex ParseStatement(TokenList* tokens, AST* ast, u8* syntaxError);
ASTNodeIndex ParseReturn(TokenList* tokens, AST* ast, u8* syntaxError);
ASTNodeIndex ParseIf(TokenList* tokens, AST* ast, u8* syntaxError);
ASTNodeIndex ParseWhile(TokenList* tokens, AST* ast, u8* syntaxError);
ASTNodeIndex ParseFor(TokenList* tokens, AST* ast, u8* syntaxError);
//ASTNodeIndex ParseExpression(TokenList* tokens, AST* ast, u8* syntaxError);
ASTNodeIndex ParseCellRef(TokenList* tokens, AST* ast, u8* syntaxError);
ASTNodeIndex ParseID(TokenList* tokens, AST* ast, u8* syntaxError);
ASTNodeIndex ParseLiteral(TokenList* tokens, AST* ast, u8* syntaxError);
ASTNodeIndex ParseDatatype(TokenList* tokens, AST* ast, u8* syntaxError);
ASTNodeIndex ParseUnit(TokenList* tokens, AST* ast, u8* syntaxError);
ASTNodeIndex ParseFunctionCall(u32 cellRef, TokenList* tokens, AST* ast,
					  u8* syntaxError);
ASTNodeIndex ParseFunctionArgs(TokenList* tokens, AST* ast, u8* syntaxError);

ASTNodeIndex ParseExpression(TokenList* tokens, AST* ast, u8* syntaxError);
ASTNodeIndex ParseExpression2(TokenList* tokens, AST* ast, u8* syntaxError, ASTNodeIndex inNode);
ASTNodeIndex ParseSummation(TokenList* tokens, AST* ast, u8* syntaxError);
ASTNodeIndex ParseSummation2(TokenList* tokens, AST* ast, u8* syntaxError, ASTNodeIndex inNode);
ASTNodeIndex ParseTerm(TokenList* tokens, AST* ast, u8* syntaxError);
ASTNodeIndex ParseTerm2(TokenList* tokens, AST* ast, u8* syntaxError, ASTNodeIndex inNode);
ASTNodeIndex ParseReference(TokenList* tokens, AST* ast, u8* syntaxError);
ASTNodeIndex ParseReference2(TokenList* tokens, AST* ast, u8* syntaxError, ASTNodeIndex inNode);
ASTNodeIndex ParseAbsolute(TokenList* tokens, AST* ast, u8* syntaxError);
ASTNodeIndex ParseAbsolute2(TokenList* tokens, AST* ast, u8* syntaxError, ASTNodeIndex inNode);


AST BuildASTFromTokens(TokenList* tokens, Allocator allocator) {
	AST ast = {
        .mem = allocator
    };

	u8 syntaxError = 0;
	(void)!ParseHeader(tokens, &ast, &syntaxError);

	if (syntaxError) {
		ASTFree(&ast);

		return (AST){0};
	}
	
	return ast;
}

ASTNodeIndex ParseHeader(TokenList* tokens, AST* ast, u8* syntaxError) {
	ExpectToken(tokens, TOKEN_CHAR_EQUALS);
	Token* nextToken = ConsumeToken(tokens);
	CheckNull(nextToken);
	if (nextToken->type == TOKEN_CHAR_OPEN_PAREN) {
		ASTNodeIndex headerArgs = ParseHeaderArgs(tokens, ast, syntaxError);
		CheckSyntaxError();
		ASTNodeIndex mainBlock = ParseBlock(tokens, ast, syntaxError);
		CheckSyntaxError();

		return ASTCreateNode(ast, AST_HEADER, headerArgs, mainBlock, EPS);
	} else {
		return ParseBlock(tokens, ast, syntaxError);
	}
}

// This doesn't expect an open parenthesis token because the caller already
// consumed it before calling
ASTNodeIndex ParseHeaderArgs(TokenList* tokens, AST* ast, u8* syntaxError) {
	Token* nextToken = ConsumeToken(tokens);
	CheckNull(nextToken);
	if (nextToken->type == TOKEN_CHAR_CLOSE_PAREN) {
		return EPS;
	}
	UnconsumeToken(tokens);
	ASTNodeIndex identifier = ParseID(tokens, ast, syntaxError);
	CheckSyntaxError();
	ExpectToken(tokens, TOKEN_CHAR_COLON);
	ASTNodeIndex type = ParseDatatype(tokens, ast, syntaxError);
	CheckSyntaxError();
	nextToken = ConsumeToken(tokens);
	CheckNull(nextToken);
	ASTNodeIndex nextArgs;
	if (nextToken->type == TOKEN_CHAR_COMMMA) {
		nextArgs = ParseHeaderArgs(tokens, ast, syntaxError);
		CheckSyntaxError();
	} else if (nextToken->type == TOKEN_CHAR_CLOSE_PAREN) {
		nextArgs = EPS;
	} else {
		ThrowUnexpectedTokenError(nextToken);
	}

	return ASTCreateNode(ast, AST_HEADER_ARGS, identifier, type, nextArgs);
}

ASTNodeIndex ParseBlock(TokenList* tokens, AST* ast, u8* syntaxError) {
	ASTNodeIndex statement = ParseStatement(tokens, ast, syntaxError);
	CheckSyntaxError();
	if (statement == EPS) {
		return EPS;
	}
	ASTNodeIndex continueBlock = ParseBlock(tokens, ast, syntaxError);
	CheckSyntaxError();
	if (continueBlock == EPS) {
		return statement;
	}

	return ASTCreateNode(ast, AST_SEQ, statement, continueBlock, EPS);
}

ASTNodeIndex ParseStatement(TokenList* tokens, AST* ast, u8* syntaxError) {
	Token* nextToken = ConsumeToken(tokens);
	if (nextToken == NULL) {
		return EPS;
	}
	ASTNodeIndex tmp;
	switch (nextToken->type) {
	case TOKEN_CHAR_SEMICOLON:
		return EPS;
	case TOKEN_CHAR_CLOSE_BRACE:
		return EPS;
	case TOKEN_KEYWORD_IF:
		return ParseIf(tokens, ast, syntaxError);
	case TOKEN_KEYWORD_WHILE:
		return ParseWhile(tokens, ast, syntaxError);
	case TOKEN_KEYWORD_FOR:
		return ParseFor(tokens, ast, syntaxError);
	case TOKEN_KEYWORD_RETURN:
		tmp = ParseReturn(tokens, ast, syntaxError);
		CheckSyntaxError();
		ExpectToken(tokens, TOKEN_CHAR_SEMICOLON);
		return tmp;
		break;
	case TOKEN_CHAR_OPEN_BRACE:
		tmp = ParseBlock(tokens, ast, syntaxError);
		CheckSyntaxError();
		ExpectToken(tokens, TOKEN_CHAR_CLOSE_BRACE);
		return tmp;
		break;
	default:
		UnconsumeToken(tokens);
		ASTNodeIndex expression = ParseExpression(tokens, ast, syntaxError);
		CheckSyntaxError();
		ExpectToken(tokens, TOKEN_CHAR_SEMICOLON);
		return expression;
		break;
	}
}

// This doesn't expect a return token because the caller already consumed it
// before calling
ASTNodeIndex ParseReturn(TokenList* tokens, AST* ast, u8* syntaxError) {
	ASTNodeIndex returnValue = ParseExpression(tokens, ast, syntaxError);
	CheckSyntaxError();
	return ASTCreateNode(ast, AST_RETURN, returnValue, EPS, EPS);
}

// This doesn't expect an if token because the caller already consumed it before
// calling
ASTNodeIndex ParseIf(TokenList* tokens, AST* ast, u8* syntaxError) {
	ExpectToken(tokens, TOKEN_CHAR_OPEN_PAREN);
	ASTNodeIndex condition = ParseExpression(tokens, ast, syntaxError);
	CheckSyntaxError();
	ExpectToken(tokens, TOKEN_CHAR_CLOSE_PAREN);
	ASTNodeIndex body = ParseStatement(tokens, ast, syntaxError);
	CheckSyntaxError();

	Token* nextToken = ConsumeToken(tokens);
	CheckNull(nextToken);
	ASTNodeIndex elseBody;
	if (nextToken != NULL) {
		if (nextToken->type == TOKEN_KEYWORD_ELSE) {
			elseBody = ParseStatement(tokens, ast, syntaxError);
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
ASTNodeIndex ParseWhile(TokenList* tokens, AST* ast, u8* syntaxError) {
	ExpectToken(tokens, TOKEN_CHAR_OPEN_PAREN);
	ASTNodeIndex condition = ParseExpression(tokens, ast, syntaxError);
	CheckSyntaxError();
	ExpectToken(tokens, TOKEN_CHAR_CLOSE_PAREN);
	ASTNodeIndex body = ParseStatement(tokens, ast, syntaxError);
	CheckSyntaxError();

	return ASTCreateNode(ast, AST_WHILE, condition, body, EPS);
}

// This doesn't expect a for token because the caller already consumed it before
// calling
ASTNodeIndex ParseFor(TokenList* tokens, AST* ast, u8* syntaxError) {
	ExpectToken(tokens, TOKEN_CHAR_OPEN_PAREN);
	ASTNodeIndex iterator = ParseID(tokens, ast, syntaxError);
	CheckSyntaxError();
	ExpectToken(tokens, TOKEN_CHAR_COMMMA);
	ASTNodeIndex range = ParseExpression(tokens, ast, syntaxError);
	CheckSyntaxError();
	ExpectToken(tokens, TOKEN_CHAR_CLOSE_PAREN);
	ASTNodeIndex body = ParseStatement(tokens, ast, syntaxError);
	CheckSyntaxError();

	return ASTCreateNode(ast, AST_FOR, iterator, range, body);
}

// This is temporary expression parsing. It's not very elegant and it doesn't
// follow operator precedence. Eli has said that he wanted to write proper
// expression parsing.
// clarise TODO: replace this

ASTNodeIndex ParseExpression(TokenList* tokens, AST* ast, u8* syntaxError){
	UnconsumeToken(tokens);
    Token* peek = PeekToken(tokens);
    if (peek->type == TOKEN_LITERAL_INT || peek->type == TOKEN_LITERAL_FLOAT ||
			peek->type == TOKEN_LITERAL_STRING || peek->type == TOKEN_CHAR_OPEN_PAREN){
        ASTNodeIndex node = ParseSummation(tokens, ast, syntaxError);
        ASTNodeIndex node2 = ParseExpression2(tokens, ast, syntaxError, node);
		return node2;
    }
	return EPS;
}
ASTNodeIndex ParseExpression2(TokenList* tokens, AST* ast, u8* syntaxError,
		ASTNodeIndex inNode){
    Token* peek = PeekToken(tokens);
    if (peek->type == TOKEN_CHAR_EQUALS){
        ExpectToken(tokens, TOKEN_CHAR_EQUALS);
        ASTNodeIndex node = ParseSummation(tokens, ast, syntaxError);
		ASTNodeIndex curr = ASTCreateNode(ast, AST_ASSIGN_VALUE, inNode, node, EPS);
        ASTNodeIndex node2 = ParseExpression2(tokens, ast, syntaxError, curr);
        return node2;
    }
	else if (peek->type == TOKEN_CHAR_SEMICOLON || peek->type == TOKEN_CHAR_CLOSE_PAREN){
        return inNode;
    }
	else {
		return EPS;
	}
}
ASTNodeIndex ParseSummation(TokenList* tokens, AST* ast, u8* syntaxError){
    Token* peek = PeekToken(tokens);
    if (peek->type == TOKEN_LITERAL_INT || peek->type == TOKEN_LITERAL_FLOAT || peek->type == TOKEN_LITERAL_STRING || peek->type == TOKEN_CHAR_OPEN_PAREN){
		ASTNodeIndex node = ParseTerm(tokens, ast, syntaxError);
		ASTNodeIndex node2 = ParseSummation2(tokens, ast, syntaxError, node);
		return node2;
    }
}
ASTNodeIndex ParseSummation2(TokenList* tokens, AST* ast, u8* syntaxError, ASTNodeIndex inNode){
    Token* peek = PeekToken(tokens);
    if (peek->type == TOKEN_CHAR_PLUS){
        ExpectToken(tokens, TOKEN_CHAR_PLUS);
        ASTNodeIndex node = ParseTerm(tokens, ast, syntaxError);
		ASTNodeIndex curr = ASTCreateNode(ast, AST_ADD, inNode, node, EPS);
        ASTNodeIndex node2 = ParseSummation2(tokens, ast, syntaxError, curr);
        return node2;
    }
    if (peek->type == TOKEN_CHAR_MINUS){
        ExpectToken(tokens, TOKEN_CHAR_MINUS);
        ASTNodeIndex node = ParseTerm(tokens, ast, syntaxError);
		ASTNodeIndex curr = ASTCreateNode(ast, AST_SUB, inNode, node, EPS);
        ASTNodeIndex node2 = ParseSummation2(tokens, ast, syntaxError, curr);
        return node2;
    }
    if (peek->type == TOKEN_CHAR_SEMICOLON || peek->type == TOKEN_CHAR_CLOSE_PAREN || peek->type == TOKEN_CHAR_EQUALS){
        return inNode;
    }
}
ASTNodeIndex ParseTerm(TokenList* tokens, AST* ast, u8* syntaxError){
    Token* peek = PeekToken(tokens);
    if (peek->type == TOKEN_LITERAL_INT || peek->type == TOKEN_LITERAL_FLOAT || peek->type == TOKEN_LITERAL_STRING || peek->type == TOKEN_CHAR_OPEN_PAREN){
        ASTNodeIndex node = ParseReference(tokens, ast, syntaxError);
        ASTNodeIndex node2 = ParseTerm2(tokens, ast, syntaxError, node);
        return node2;
    }
}
ASTNodeIndex ParseTerm2(TokenList* tokens, AST* ast, u8* syntaxError, ASTNodeIndex inNode){
    Token* peek = PeekToken(tokens);
    if (peek->type == TOKEN_CHAR_ASTERISK){
        ExpectToken(tokens, TOKEN_CHAR_ASTERISK);
        ASTNodeIndex node = ParseReference(tokens, ast, syntaxError);
		ASTNodeIndex curr = ASTCreateNode(ast, AST_MUL, inNode, node, EPS);
        ASTNodeIndex node2 = ParseTerm2(tokens, ast, syntaxError, curr);
        return node2;
    }
    if (peek->type == TOKEN_CHAR_SLASH){
        ExpectToken(tokens, TOKEN_CHAR_SLASH);
        ASTNodeIndex node = ParseReference(tokens, ast, syntaxError);
		ASTNodeIndex curr = ASTCreateNode(ast, AST_DIV, inNode, node, EPS);
        ASTNodeIndex node2 = ParseTerm2(tokens, ast, syntaxError, curr);
        return node2;
    }
    if (peek->type == TOKEN_CHAR_SEMICOLON || peek->type == TOKEN_CHAR_CLOSE_PAREN || peek->type == TOKEN_CHAR_EQUALS || peek->type == TOKEN_CHAR_PLUS || peek->type == TOKEN_CHAR_MINUS){
        return inNode;
    }
}
ASTNodeIndex ParseReference(TokenList* tokens, AST* ast, u8* syntaxError){
    Token* peek = PeekToken(tokens);
    if (peek->type == TOKEN_LITERAL_INT || peek->type == TOKEN_LITERAL_FLOAT || peek->type == TOKEN_LITERAL_STRING || peek->type == TOKEN_CHAR_OPEN_PAREN){
        ASTNodeIndex node = ParseAbsolute(tokens, ast, syntaxError);
        ASTNodeIndex node2 = ParseReference2(tokens, ast, syntaxError, node);
        return node2;
    }
}
ASTNodeIndex ParseReference2(TokenList* tokens, AST* ast, u8* syntaxError, ASTNodeIndex inNode){
    Token* peek = PeekToken(tokens);
    if (peek->type == TOKEN_CHAR_COLON){
        ExpectToken(tokens, TOKEN_CHAR_COLON);
        ASTNodeIndex node = ParseAbsolute(tokens, ast, syntaxError);
		ASTNodeIndex curr = ASTCreateNode(ast, AST_RANGE, inNode, node, EPS);
        ASTNodeIndex node2 = ParseReference2(tokens, ast, syntaxError, curr);
        return node2;
    }
    if (peek->type == TOKEN_CHAR_SEMICOLON || peek->type == TOKEN_CHAR_CLOSE_PAREN || peek->type == TOKEN_CHAR_EQUALS || peek->type == TOKEN_CHAR_PLUS || peek->type == TOKEN_CHAR_MINUS || peek->type == TOKEN_CHAR_ASTERISK || peek->type == TOKEN_CHAR_SLASH){
        return inNode;
    }
}

ASTNodeIndex ParseAbsolute(TokenList* tokens, AST* ast, u8* syntaxError){
    Token* peek = PeekToken(tokens);
    if (peek->type == TOKEN_LITERAL_INT || peek->type == TOKEN_LITERAL_FLOAT || peek->type == TOKEN_LITERAL_STRING || peek->type == TOKEN_CHAR_OPEN_PAREN){
        ASTNodeIndex node = ParseUnit(tokens, ast, syntaxError);
        return node;
    }
    if (peek->type == TOKEN_CHAR_OCTOTHORPE){
        ExpectToken(tokens, TOKEN_CHAR_OCTOTHORPE);
        ASTNodeIndex node = ParseUnit(tokens, ast, syntaxError);
        return ASTCreateNode(ast, AST_COORD_TRANSFORM, node, EPS, EPS);
    }
}

// This doesn't expect an open bracket token because the caller already consumed
// it before calling
ASTNodeIndex ParseCellRef(TokenList* tokens, AST* ast, u8* syntaxError) {
	ASTNodeIndex x = ParseExpression(tokens, ast, syntaxError);
	CheckSyntaxError();
	ExpectToken(tokens, TOKEN_CHAR_COMMMA);
	ASTNodeIndex y = ParseExpression(tokens, ast, syntaxError);
	CheckSyntaxError();
	ExpectToken(tokens, TOKEN_CHAR_CLOSE_BRACKET);

	return ASTCreateNode(ast, AST_GET_CELL_REF, x, y, EPS);
}

ASTNodeIndex ParseID(TokenList* tokens, AST* ast, u8* syntaxError) {
	Token* id = ConsumeToken(tokens);
	CheckNull(id);

	ASTNodeIndex new_node_index = ASTCreateNode(ast, AST_ID, EPS, EPS, EPS);

	ast->nodes[new_node_index].vt = V_INT;
	ast->nodes[new_node_index].data.i = id->data.i;

	return new_node_index;
}

// TODO: Actually get literal values from token string (this should probably be
// a function in the tokenizer that is called here)
// clarise TODO: replace literal values with literal->data once rebased with tokenizer fixes
ASTNodeIndex ParseLiteral(TokenList* tokens, AST* ast, u8* syntaxError) {
	Token* literal = ConsumeToken(tokens);
	CheckNull(literal);

	ASTNodeIndex new_node_index = ASTCreateNode(ast, AST_INVALID, EPS, EPS, EPS);

	switch (literal->type) {
	case (TOKEN_LITERAL_INT):
		ast->nodes[new_node_index].op = AST_INT_LITERAL;
		ast->nodes[new_node_index].vt = V_INT;
		ast->nodes[new_node_index].data.i = 4; // TODO: Get literal values
		break;
	case (TOKEN_LITERAL_FLOAT):
		ast->nodes[new_node_index].op = AST_FLOAT_LITERAL;
		ast->nodes[new_node_index].vt = V_FLOAT;
		ast->nodes[new_node_index].data.f = 4.2; // TODO: Get literal values
		break;
	case (TOKEN_LITERAL_STRING):
		ast->nodes[new_node_index].op = AST_INVALID; // TODO: Add strings
		ast->nodes[new_node_index].vt = V_INT;		 // Interned string
		ast->nodes[new_node_index].data.i = 0;		 // TODO: Get literal values
		break;
	default:
		break;
	}

	return new_node_index;
}

ASTNodeIndex ParseDatatype(TokenList* tokens, AST* ast, u8* syntaxError) {
	Token* dataType = ConsumeToken(tokens);
	CheckNull(dataType);

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

ASTNodeIndex ParseUnit(TokenList* tokens, AST* ast, u8* syntaxError) {
	Token* unit = ConsumeToken(tokens);
	CheckNull(unit);

	ASTNodeIndex tmp;
	switch (unit->type) {
	case TOKEN_LITERAL_INT:
		UnconsumeToken(tokens);
		return ParseLiteral(tokens, ast, syntaxError);
	case TOKEN_LITERAL_FLOAT:
		UnconsumeToken(tokens);
		return ParseLiteral(tokens, ast, syntaxError);
	case TOKEN_LITERAL_STRING:
		UnconsumeToken(tokens);
		return ParseLiteral(tokens, ast, syntaxError);
	case TOKEN_CHAR_OPEN_PAREN:
		tmp = ParseExpression(tokens, ast, syntaxError);
		CheckSyntaxError();
		ExpectToken(tokens, TOKEN_CHAR_CLOSE_PAREN);
		return tmp;
	default:
		ThrowUnexpectedTokenError(unit);
		return EPS;
	}
}

// The function should have already been consumed because of how expression
// parsing currently works
ASTNodeIndex ParseFunctionCall(ASTNodeIndex cellRef, TokenList* tokens, AST* ast,
					  u8* syntaxError) {
	ASTNodeIndex args = ParseFunctionArgs(tokens, ast, syntaxError);
	CheckSyntaxError();
	return ASTCreateNode(ast, AST_CALL, cellRef, args, EPS);
}

// This doesn't expect an open parenthesis token because the caller already
// consumed it before calling
ASTNodeIndex ParseFunctionArgs(TokenList* tokens, AST* ast, u8* syntaxError) {
	Token* nextToken = ConsumeToken(tokens);
	CheckNull(nextToken);
	if (nextToken->type == TOKEN_CHAR_CLOSE_PAREN) {
		return EPS;
	}
	UnconsumeToken(tokens);
	ASTNodeIndex arg = ParseID(tokens, ast, syntaxError);
	CheckSyntaxError();
	nextToken = ConsumeToken(tokens);
	CheckNull(nextToken);
	ASTNodeIndex nextArgs;
	if (nextToken->type == TOKEN_CHAR_COMMMA) {
		nextArgs = ParseFunctionArgs(tokens, ast, syntaxError);
		CheckSyntaxError();
	} else if (nextToken->type == TOKEN_CHAR_CLOSE_PAREN) {
		nextArgs = EPS;
	} else {
		ThrowUnexpectedTokenError(nextToken);
		return EPS;
	}
	return ASTCreateNode(ast, AST_FUNC_ARGS, arg, nextArgs, EPS);
}
