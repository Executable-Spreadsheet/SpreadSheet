#include <libparasheet/tokenizer.h>
#include <libparasheet/tokenizer_types.h>
#include <util/util.h>
#include <stdio.h>
#include <string.h>

const char* TokenTypeToString(TokenType type) {
    switch (type) {
        case TOKEN_ID: return "TOKEN_ID";
        case TOKEN_KEYWORD_IF: return "TOKEN_KEYWORD_IF";
        case TOKEN_KEYWORD_ELSE: return "TOKEN_KEYWORD_ELSE";
        case TOKEN_KEYWORD_RETURN: return "TOKEN_KEYWORD_RETURN";
        case TOKEN_KEYWORD_LET: return "TOKEN_KEYWORD_LET";
        case TOKEN_KEYWORD_WHILE: return "TOKEN_KEYWORD_WHILE";
        case TOKEN_KEYWORD_FOR: return "TOKEN_KEYWORD_FOR";
        case TOKEN_KEYWORD_INT: return "TOKEN_KEYWORD_INT";
        case TOKEN_KEYWORD_FLOAT: return "TOKEN_KEYWORD_FLOAT";
        case TOKEN_KEYWORD_STRING: return "TOKEN_KEYWORD_STRING";
        case TOKEN_KEYWORD_CELL: return "TOKEN_KEYWORD_CELL";
        case TOKEN_LITERAL_INT: return "TOKEN_LITERAL_INT";
        case TOKEN_LITERAL_FLOAT: return "TOKEN_LITERAL_FLOAT";
        case TOKEN_LITERAL_STRING: return "TOKEN_LITERAL_STRING";
        case TOKEN_OP_PLUS: return "TOKEN_OP_PLUS";
        case TOKEN_OP_MINUS: return "TOKEN_OP_MINUS";
        case TOKEN_OP_TIMES: return "TOKEN_OP_TIMES";
        case TOKEN_OP_DIVIDE: return "TOKEN_OP_DIVIDE";
        case TOKEN_OP_OCTOTHORPE: return "TOKEN_OP_OCTOTHORPE";
        case TOKEN_OP_COLON: return "TOKEN_OP_COLON";
        case TOKEN_GROUPING_OPEN_PAREN: return "TOKEN_GROUPING_OPEN_PAREN";
        case TOKEN_GROUPING_CLOSE_PAREN: return "TOKEN_GROUPING_CLOSE_PAREN";
        case TOKEN_GROUPING_OPEN_BRACKET: return "TOKEN_GROUPING_OPEN_BRACKET";
        case TOKEN_GROUPING_CLOSE_BRACKET: return "TOKEN_GROUPING_CLOSE_BRACKET";
        case TOKEN_GROUPING_OPEN_BRACE: return "TOKEN_GROUPING_OPEN_BRACE";
        case TOKEN_GROUPING_CLOSE_BRACE: return "TOKEN_GROUPING_CLOSE_BRACE";
        case TOKEN_INVALID: return "TOKEN_INVALID";
        default: return "TOKEN_UNKNOWN";
    }
}

int main() {
    Allocator allocator = GlobalAllocatorCreate();

    // Only tokenize tokens explicitly listed in tokenizer_types.h
    const char* input =
//        "if else return let while for int float string cell "
//        "123 \"HELLO\" 
        "+ - * / # : ( ) [ ] { }";

    TokenList* tokens = Tokenize(input, allocator);

    log("Token count: %d", tokens->size);
    for (u32 i = 0; i < tokens->size; i++) {
        Token* t = &tokens->tokens[i];

        print(stdout, "Token %2d: %-30s", i, TokenTypeToString(t->type));

        if (t->string.size > 0) {
            print(stdout, "  '%.*s'", t->string.size, t->string.data);
        }

        if (t->type == TOKEN_ID) {
            print(stdout, "  [symbolTableIndex: %d]", t->symbolTableIndex);
        }

        print(stdout, "\n");
    }

    DestroyTokenList(&tokens);
    return 0;
}
