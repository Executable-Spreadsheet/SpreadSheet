#ifndef PS_TOKENIZER_H
#define PS_TOKENIZER_H

#include <libparasheet/tokenizer_types.h>

// Takes in raw source code and returns a TokenList of tokens
TokenList* Tokenize(const char* source, StringTable* table, Allocator allocator);

#endif // PS_TOKENIZER_H
