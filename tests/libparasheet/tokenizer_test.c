#include <libparasheet/tokenizer.h>
#include <util/util.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Optional helper
static const char* TokenTypeToString(TokenType t) {
    switch (t) {
        case TOKEN_ID: return "TOKEN_ID";
        case TOKEN_KEYWORD_IF: return "TOKEN_KEYWORD_IF";
        case TOKEN_LITERAL_INT: return "TOKEN_LITERAL_INT";
        case TOKEN_OP_PLUS: return "TOKEN_OP_PLUS";
        case TOKEN_GROUPING_OPEN_PAREN: return "TOKEN_GROUPING_OPEN_PAREN";
        case TOKEN_GROUPING_CLOSE_PAREN: return "TOKEN_GROUPING_CLOSE_PAREN";
        case TOKEN_INVALID: return "TOKEN_INVALID";
        default: return "UNKNOWN_TOKEN";
    }
}

int main() {
    Allocator allocator = GlobalAllocatorCreate();

    const char* input = "if (x + 42)";

    TokenList* tokens = Tokenize(input, allocator);

    for (u32 i = 0; i < tokens->size; i++) {
        Token* t = &tokens->tokens[i];
        printf("Token %2d: %-20s | Value: %.*s\n",
               i,
               TokenTypeToString(t->type),
               t->string.size,
               t->string.data);
    }

    DestroyTokenList(&tokens);
    return 0;
}
