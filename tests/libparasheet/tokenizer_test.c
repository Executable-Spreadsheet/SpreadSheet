#include <libparasheet/tokenizer.h>
#include <util/util.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Allocator for testing
static void* test_allocator_func(u64 oldsize, u64 newsize, void* ptr, void* ctx) {
    (void)ctx;
    if (oldsize == 0 && newsize > 0) {
        return malloc(newsize);
    } else if (newsize == 0 && ptr != NULL) {
        free(ptr);
        return NULL;
    } else {
        return realloc(ptr, newsize);
    }
}

static Allocator make_test_allocator() {
    return (Allocator){
        .a = test_allocator_func,
        .ctx = NULL
    };
}

// Optional helper
static const char* TokenTypeToString(TokenType t) {
    switch (t) {
        case TOKEN_ID: return "TOKEN_ID";
        case TOKEN_KEYWORD_IF: return "TOKEN_KEYWORD_IF";
        case TOKEN_LITERAL_INT: return "TOKEN_LITERAL_INT";
        case TOKEN_OP_PLUS: return "TOKEN_OP_PLUS";
        case TOKEN_GROUPING_OPEN_PAREN: return "TOKEN_GROUPING_OPEN_PAREN";
        case TOKEN_INVALID: return "TOKEN_INVALID";
        default: return "UNKNOWN_TOKEN";
    }
}

int main() {
    Allocator allocator = make_test_allocator();

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
