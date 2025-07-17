#include "util/util.h"
#include <libparasheet/lib_internal.h>
#include <libparasheet/tokenizer_types.h>

#define EPS UINT32_MAX

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
		 found->lineNumber, getTokenErrorString(expected), found->string);
}

void PrintNullTokenErrror(Token* prev) {
	warn("Syntax error on line %d: Expected token after \"%s\"",
		 prev->lineNumber, prev->string);
}

void PrintUnexpectedTokenError(Token* unexpected) {
	// We probably want to have a better way to handle errors (so the user can
	// see errors in the ncurses TUI)
	warn("Syntax error on line %d: Unexpected token \"%s\"",
		 unexpected->lineNumber, unexpected->string);
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

void ConsumeUntilToken(TokenList* tokens, TokenType expected) {
	while (ConsumeToken(tokens)->type != expected)
		;
}

u32 ParseHeader(TokenList* tokens, AST* ast, u8* syntaxError);
u32 ParseHeaderArgs(TokenList* tokens, AST* ast, u8* syntaxError);
u32 ParseBlock(TokenList* tokens, AST* ast, u8* syntaxError);
u32 ParseStatement(TokenList* tokens, AST* ast, u8* syntaxError);
u32 ParseReturn(TokenList* tokens, AST* ast, u8* syntaxError);
u32 ParseIf(TokenList* tokens, AST* ast, u8* syntaxError);
u32 ParseWhile(TokenList* tokens, AST* ast, u8* syntaxError);
u32 ParseFor(TokenList* tokens, AST* ast, u8* syntaxError);
u32 ParseExpression(TokenList* tokens, AST* ast, u8* syntaxError);
u32 ParseCellRef(TokenList* tokens, AST* ast, u8* syntaxError);
u32 ParseID(TokenList* tokens, AST* ast, u8* syntaxError);
u32 ParseLiteral(TokenList* tokens, AST* ast, u8* syntaxError);
u32 ParseDatatype(TokenList* tokens, AST* ast, u8* syntaxError);
u32 ParseUnit(TokenList* tokens, AST* ast, u8* syntaxError);
u32 ParseFunctionCall(u32 cellRef, TokenList* tokens, AST* ast,
					  u8* syntaxError);
u32 ParseFunctionArgs(TokenList* tokens, AST* ast, u8* syntaxError);

AST* BuildASTFromTokens(TokenList* tokens, Allocator allocator) {
	AST* ast = Alloc(allocator, sizeof(AST));
	ast->mem = allocator;
	ast->nodes = NULL;
	ast->size = 0;
	ast->cap = 0;

	u8 syntaxError = 0;
	(void)!ParseHeader(tokens, ast, &syntaxError);

	if (syntaxError) {
		ASTFree(ast);
		Free(allocator, ast, sizeof(AST));

		return NULL;
	}

	return ast;
}

u32 ParseHeader(TokenList* tokens, AST* ast, u8* syntaxError) {
	ExpectToken(tokens, TOKEN_CHAR_EQUALS);
	Token* nextToken = ConsumeToken(tokens);
	CheckNull(nextToken);
	if (nextToken->type == TOKEN_CHAR_OPEN_PAREN) {
		u32 headerArgs = ParseHeaderArgs(tokens, ast, syntaxError);
		CheckSyntaxError();
		u32 mainBlock = ParseBlock(tokens, ast, syntaxError);
		CheckSyntaxError();

		return ASTCreateNode(ast, AST_HEADER, headerArgs, mainBlock, EPS);
	} else {
		return ParseBlock(tokens, ast, syntaxError);
	}
}

u32 ParseHeaderArgs(TokenList* tokens, AST* ast, u8* syntaxError) {
	Token* nextToken = ConsumeToken(tokens);
	CheckNull(nextToken);
	if (nextToken->type == TOKEN_CHAR_CLOSE_PAREN) {
		return EPS;
	}
	UnconsumeToken(tokens);
	u32 identifier = ParseID(tokens, ast, syntaxError);
	CheckSyntaxError();
	ExpectToken(tokens, TOKEN_CHAR_COLON);
	u32 type = ParseDatatype(tokens, ast, syntaxError);
	CheckSyntaxError();
	nextToken = ConsumeToken(tokens);
	CheckNull(nextToken);
	u32 nextArgs;
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

u32 ParseBlock(TokenList* tokens, AST* ast, u8* syntaxError) {
	u32 statement = ParseStatement(tokens, ast, syntaxError);
	CheckSyntaxError();
	if (statement == EPS) {
		return EPS;
	}
	u32 continueBlock = ParseBlock(tokens, ast, syntaxError);
	CheckSyntaxError();
	if (continueBlock == EPS) {
		return statement;
	}

	return ASTCreateNode(ast, AST_SEQ, statement, continueBlock, EPS);
}

u32 ParseStatement(TokenList* tokens, AST* ast, u8* syntaxError) {
	Token* nextToken = ConsumeToken(tokens);
	if (nextToken == NULL) {
		return EPS;
	}
	u32 tmp;
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
		u32 expression = ParseExpression(tokens, ast, syntaxError);
		CheckSyntaxError();
		ExpectToken(tokens, TOKEN_CHAR_SEMICOLON);
		return expression;
		break;
	}
}

u32 ParseReturn(TokenList* tokens, AST* ast, u8* syntaxError) {
	u32 returnValue = ParseExpression(tokens, ast, syntaxError);
	CheckSyntaxError();
	return ASTCreateNode(ast, AST_RETURN, returnValue, EPS, EPS);
}

u32 ParseIf(TokenList* tokens, AST* ast, u8* syntaxError) {
	ExpectToken(tokens, TOKEN_CHAR_OPEN_PAREN);
	u32 condition = ParseExpression(tokens, ast, syntaxError);
	CheckSyntaxError();
	ExpectToken(tokens, TOKEN_CHAR_CLOSE_PAREN);
	u32 body = ParseStatement(tokens, ast, syntaxError);
	CheckSyntaxError();

	Token* nextToken = ConsumeToken(tokens);
	CheckNull(nextToken);
	u32 elseBody;
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

u32 ParseWhile(TokenList* tokens, AST* ast, u8* syntaxError) {
	ExpectToken(tokens, TOKEN_CHAR_OPEN_PAREN);
	u32 condition = ParseExpression(tokens, ast, syntaxError);
	CheckSyntaxError();
	ExpectToken(tokens, TOKEN_CHAR_CLOSE_PAREN);
	u32 body = ParseStatement(tokens, ast, syntaxError);
	CheckSyntaxError();

	return ASTCreateNode(ast, AST_WHILE, condition, body, EPS);
}

u32 ParseFor(TokenList* tokens, AST* ast, u8* syntaxError) {
	ExpectToken(tokens, TOKEN_CHAR_OPEN_PAREN);
	u32 iterator = ParseID(tokens, ast, syntaxError);
	CheckSyntaxError();
	ExpectToken(tokens, TOKEN_CHAR_COMMMA);
	u32 range = ParseExpression(tokens, ast, syntaxError);
	CheckSyntaxError();
	ExpectToken(tokens, TOKEN_CHAR_CLOSE_PAREN);
	u32 body = ParseStatement(tokens, ast, syntaxError);
	CheckSyntaxError();

	return ASTCreateNode(ast, AST_FOR, iterator, range, body);
}

// This is temporary expression parsing. It's not very elegant and it doesn't
// follow operator precedence. Eli has said that he wanted to write proper
// expression parsing.
u32 ParseExpression(TokenList* tokens, AST* ast, u8* syntaxError) {
	enum ExpressionType {
		UNKNOWN,
		UNIT,
		BINARY_OPERATION,
		UNARY_OPERATION,
		ASSIGNMENT,
		DECLARATION
	};

	enum ExpressionType type = UNKNOWN;

	Token* token1 = ConsumeToken(tokens);
	Token* token2;
	CheckNull(token1);

	u32 operand1;
	u32 operand2;

	switch (token1->type) {
	case TOKEN_CHAR_OPEN_PAREN:
		UnconsumeToken(tokens);
		operand1 = ParseUnit(tokens, ast, syntaxError);
		CheckSyntaxError();
		break;
	case TOKEN_LITERAL_INT:
		UnconsumeToken(tokens);
		operand1 = ParseUnit(tokens, ast, syntaxError);
		CheckSyntaxError();
		break;
	case TOKEN_LITERAL_FLOAT:
		UnconsumeToken(tokens);
		operand1 = ParseUnit(tokens, ast, syntaxError);
		CheckSyntaxError();
		break;
	case TOKEN_LITERAL_STRING:
		UnconsumeToken(tokens);
		operand1 = ParseUnit(tokens, ast, syntaxError);
		CheckSyntaxError();
		break;
	case TOKEN_ID:
		UnconsumeToken(tokens);
		operand1 = ParseID(tokens, ast, syntaxError);
		CheckSyntaxError();
		token2 = ConsumeToken(tokens);
		CheckNull(token2);
		if (token2->type == TOKEN_CHAR_EQUALS) {
			type = ASSIGNMENT;
		} else if (token2->type == TOKEN_CHAR_OPEN_PAREN) {
			operand1 = ParseFunctionCall(operand1, tokens, ast, syntaxError);
			CheckSyntaxError();
		} else {
			UnconsumeToken(tokens);
		}
		break;
	case TOKEN_CHAR_OPEN_BRACKET:
		operand1 = ParseCellRef(tokens, ast, syntaxError);
		CheckSyntaxError();
		token2 = ConsumeToken(tokens);
		CheckNull(token2);
		if (token2->type == TOKEN_CHAR_EQUALS) {
			type = ASSIGNMENT;
		} else if (token2->type == TOKEN_CHAR_OPEN_PAREN) {
			operand1 = ParseFunctionCall(operand1, tokens, ast, syntaxError);
			CheckSyntaxError();
		} else {
			UnconsumeToken(tokens);
		}
		break;
	case TOKEN_CHAR_OCTOTHORPE:
		type = UNARY_OPERATION;
		break;
	case TOKEN_KEYWORD_LET:
		type = DECLARATION;
		break;
	default:
		ThrowUnexpectedTokenError(token1);
		break;
	}

	if (type == UNARY_OPERATION) {
		operand1 = ParseExpression(tokens, ast, syntaxError);
		CheckSyntaxError();
		// We currently only have 1 unary operation, so we do not need a switch
		// statement here
		return ASTCreateNode(ast, AST_COORD_TRANSFORM, operand1, EPS, EPS);
	} else if (type == ASSIGNMENT) {
		operand2 = ParseExpression(tokens, ast, syntaxError);
		CheckSyntaxError();
		return ASTCreateNode(ast, AST_ASSIGN_VALUE, operand1, operand2, EPS);
	} else if (type == DECLARATION) {
		operand1 = ParseID(tokens, ast, syntaxError);
		CheckSyntaxError();
		ExpectToken(tokens, TOKEN_CHAR_COLON);
		token2 = ConsumeToken(tokens);
		CheckNull(token2);
		if (token2->type == TOKEN_CHAR_EQUALS) {
			operand2 = EPS;
		} else {
			UnconsumeToken(tokens);
			operand2 = ParseDatatype(tokens, ast, syntaxError);
			CheckSyntaxError();
			ExpectToken(tokens, TOKEN_CHAR_EQUALS);
		}
		u32 assignmentValue = ParseExpression(tokens, ast, syntaxError);
		CheckSyntaxError();

		return ASTCreateNode(ast, AST_DECLARE_VARIABLE, operand1, operand2,
							 assignmentValue);
	} else {
		token2 = ConsumeToken(tokens);
		CheckNull(token2);

		ASTNodeOp operation;

		switch (token2->type) {
		case TOKEN_CHAR_PLUS:
			operation = AST_ADD;
			break;
		case TOKEN_CHAR_MINUS:
			operation = AST_SUB;
			break;
		case TOKEN_CHAR_ASTERISK:
			operation = AST_MUL;
			break;
		case TOKEN_CHAR_SLASH:
			operation = AST_DIV;
			break;
		case TOKEN_CHAR_COLON:
			operation = AST_RANGE;
			break;
		default:
			UnconsumeToken(tokens);
			return operand1;
		}

		operand2 = ParseExpression(tokens, ast, syntaxError);
		CheckSyntaxError();

		return ASTCreateNode(ast, operation, operand1, operand2, EPS);
	}
}

u32 ParseCellRef(TokenList* tokens, AST* ast, u8* syntaxError) {
	u32 x = ParseExpression(tokens, ast, syntaxError);
	CheckSyntaxError();
	ExpectToken(tokens, TOKEN_CHAR_COMMMA);
	u32 y = ParseExpression(tokens, ast, syntaxError);
	CheckSyntaxError();
	ExpectToken(tokens, TOKEN_CHAR_CLOSE_BRACKET);

	return ASTCreateNode(ast, AST_GET_CELL_REF, x, y, EPS);
}

u32 ParseID(TokenList* tokens, AST* ast, u8* syntaxError) {
	Token* id = ConsumeToken(tokens);
	CheckNull(id);

	u32 new_node_index = ASTCreateNode(ast, AST_ID, EPS, EPS, EPS);

	ast->nodes[new_node_index].vt = V_INT;
	ast->nodes[new_node_index].data.i = id->symbolTableIndex;

	return new_node_index;
}

u32 ParseLiteral(TokenList* tokens, AST* ast, u8* syntaxError) {
	Token* literal = ConsumeToken(tokens);
	CheckNull(literal);

	u32 new_node_index = ASTCreateNode(ast, AST_INVALID, EPS, EPS, EPS);

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

u32 ParseDatatype(TokenList* tokens, AST* ast, u8* syntaxError) {
	Token* dataType = ConsumeToken(tokens);
	CheckNull(dataType);

	u32 new_node_index = ASTCreateNode(ast, AST_INVALID, EPS, EPS, EPS);

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

u32 ParseUnit(TokenList* tokens, AST* ast, u8* syntaxError) {
	Token* unit = ConsumeToken(tokens);
	CheckNull(unit);

	u32 tmp;
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

u32 ParseFunctionCall(u32 cellRef, TokenList* tokens, AST* ast,
					  u8* syntaxError) {
	u32 args = ParseFunctionArgs(tokens, ast, syntaxError);
	CheckSyntaxError();
	return ASTCreateNode(ast, AST_CALL, cellRef, args, EPS);
}
u32 ParseFunctionArgs(TokenList* tokens, AST* ast, u8* syntaxError) {
	Token* nextToken = ConsumeToken(tokens);
	CheckNull(nextToken);
	if (nextToken->type == TOKEN_CHAR_CLOSE_PAREN) {
		return EPS;
	}
	UnconsumeToken(tokens);
	u32 arg = ParseID(tokens, ast, syntaxError);
	CheckSyntaxError();
	nextToken = ConsumeToken(tokens);
	CheckNull(nextToken);
	u32 nextArgs;
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
