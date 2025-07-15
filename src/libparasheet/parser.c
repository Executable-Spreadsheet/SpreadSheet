#include "util/util.h"
#include <libparasheet/lib_internal.h>
#include <libparasheet/tokenizer_types.h>

#define EPS UINT32_MAX

void ThrowSyntaxError(TokenType expected, Token* found) {
	warn("Syntax error on line (idk): Expected (we need a function to turn "
		 "this into a string) but found (we need a function to turn this into "
		 "a string)");
}

void ThrowUnexpectedTokenError(Token* unexpected) {
	warn("Unexpected token on line (idk): (we need a function to turn "
		 "this into a string)");
}

void ExpectToken(TokenList* tokens, TokenType expected) {
	Token* nextToken = ConsumeToken(tokens);
	if (nextToken->type != expected) {
		ThrowSyntaxError(expected, nextToken);
	}
}

void ConsumeUntilToken(TokenList* tokens, TokenType expected) {
	while (ConsumeToken(tokens)->type != expected)
		;
}

u32 ParseCell(TokenList* tokens, AST* ast);
u32 ParseHeader(TokenList* tokens, AST* ast);
u32 ParseHeaderArgs(TokenList* tokens, AST* ast);
u32 ParseBlock(TokenList* tokens, AST* ast);
u32 ParseStatement(TokenList* tokens, AST* ast);
u32 ParseReturn(TokenList* tokens, AST* ast);
u32 ParseIf(TokenList* tokens, AST* ast);
u32 ParseWhile(TokenList* tokens, AST* ast);
u32 ParseFor(TokenList* tokens, AST* ast);
u32 ParseExpression(TokenList* tokens, AST* ast);
u32 ParseCellRef(TokenList* tokens, AST* ast);
u32 ParseID(TokenList* tokens, AST* ast);
u32 ParseTerminal(TokenList* tokens, AST* ast);

AST* BuildASTFromTokens(TokenList* tokens, Allocator allocator) {
	AST* ast = Alloc(allocator, sizeof(AST));
	ast->mem = allocator;
	ast->nodes = NULL;
	ast->size = 0;
	ast->cap = 0;

	(void)!ParseCell(tokens, ast);

	return ast;
}

u32 ParseCell(TokenList* tokens, AST* ast) {
	u32 header = ParseHeader(tokens, ast);
	u32 mainBlock = ParseBlock(tokens, ast);

	u32 new_node_index = ASTPush(ast);
	ast->nodes[new_node_index].lchild = header;
	ast->nodes[new_node_index].rchild = mainBlock;

	return new_node_index;
}

u32 ParseHeader(TokenList* tokens, AST* ast) {
	ExpectToken(tokens, TOKEN_EQUALS);
	Token* nextToken = ConsumeToken(tokens);
	if (nextToken->type == TOKEN_GROUPING_OPEN_PAREN) {
		// tbh im kinda confused about how the grammar is supposed to work
		// here...
	}

	return 0;
}

u32 ParseHeaderArgs(TokenList* tokens, AST* ast) { todo(); }

// How does anything later know this is a block? should that be stored with the
// AST somehow?
u32 ParseBlock(TokenList* tokens, AST* ast) {
	u32 statement = ParseStatement(tokens, ast);
	u32 continueBlock = ParseBlock(tokens, ast);

	if (continueBlock == EPS) {
		return statement;
	}

	u32 new_node_index = ASTPush(ast);
	ast->nodes[new_node_index].lchild = statement;
	ast->nodes[new_node_index].rchild = continueBlock;

	return new_node_index;
}

// How does anything later know this is a statememt? should that be stored with
// the AST somehow?
u32 ParseStatement(TokenList* tokens, AST* ast) {
	Token* nextToken = ConsumeToken(tokens);
	if (nextToken == NULL) {
		return EPS;
	}
	switch (nextToken->type) {
	case TOKEN_KEYWORD_IF:
		return ParseIf(tokens, ast);
		break;
	case TOKEN_KEYWORD_WHILE:
		return ParseWhile(tokens, ast);
		break;
	case TOKEN_KEYWORD_FOR:
		return ParseFor(tokens, ast);
		break;
	case TOKEN_KEYWORD_RETURN:
		u32 returnIndex = ParseReturn(tokens, ast);
		ExpectToken(tokens, TOKEN_SEMICOLON);
		return returnIndex;
		break;
	case TOKEN_GROUPING_OPEN_BRACE:
		u32 block = ParseBlock(tokens, ast);
		ExpectToken(tokens, TOKEN_GROUPING_CLOSE_BRACE);
		return block;
		break;
	default:
		UnconsumeToken(tokens);
		u32 expression = ParseExpression(tokens, ast);
		ExpectToken(tokens, TOKEN_SEMICOLON);
		return expression;
		break;
	}
}

// How does anything later know this is a return (and thus has only one child)?
// should that be stored with the AST somehow?
u32 ParseReturn(TokenList* tokens, AST* ast) {
	u32 returnValue = ParseExpression(tokens, ast);

	u32 new_node_index = ASTPush(ast);
	ast->nodes[new_node_index].lchild = returnValue;

	return new_node_index;
}

// How does anything later know this is an if? should that be stored with the
// AST somehow?
u32 ParseIf(TokenList* tokens, AST* ast) {
	u32 condition = ParseExpression(tokens, ast);
	u32 body = ParseBlock(tokens, ast);

	u32 new_node_index = ASTPush(ast);
	ast->nodes[new_node_index].lchild = condition;
	ast->nodes[new_node_index].rchild = body;

	// How do I do ELSE? If can only have two children.

	return new_node_index;
}

// How does anything later know this is a while? should that be stored with the
// AST somehow?
u32 ParseWhile(TokenList* tokens, AST* ast) {
	u32 condition = ParseExpression(tokens, ast);
	u32 body = ParseBlock(tokens, ast);

	u32 new_node_index = ASTPush(ast);
	ast->nodes[new_node_index].lchild = condition;
	ast->nodes[new_node_index].rchild = body;

	return new_node_index;
}

// How does anything later know this is a for? should that be stored with the
// AST somehow?
u32 ParseFor(TokenList* tokens, AST* ast) {
	u32 iterator = ParseID(tokens, ast);
	u32 condition = ParseExpression(tokens, ast);

	u32 new_node_index = ASTPush(ast);
	ast->nodes[new_node_index].lchild = iterator;
	ast->nodes[new_node_index].rchild = condition;

	// How do I do the body? For can only have two children.if

	return new_node_index;
}

// How does anything later know this is an expression? should that be stored
// with the AST somehow?
u32 ParseExpression(TokenList* tokens, AST* ast) {
	todo(); // Double E infix something is to happen here
}

// How does anything later know this is a cell ref? should that be stored with
// the AST somehow?
u32 ParseCellRef(TokenList* tokens, AST* ast) {
	u32 x = ParseExpression(tokens, ast);
	u32 y = ParseExpression(tokens, ast);

	u32 new_node_index = ASTPush(ast);
	ast->nodes[new_node_index].lchild = x;
	ast->nodes[new_node_index].rchild = y;

	return new_node_index;
}

u32 ParseTerminal(TokenList* tokens, AST* ast) {
	todo(); // I think this one depends a lot on the tokenizer, so I'll wait for
			// that to be done.
}
