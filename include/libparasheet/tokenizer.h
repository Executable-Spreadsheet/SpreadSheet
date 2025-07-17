#ifndef PS_TOKENIZER_H
#define PS_TOKENIZER_H

#include <libparasheet/tokenizer_types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Takes in raw source code and returns a TokenList of tokens
TokenList* Tokenize(const char* source, Allocator allocator);

#ifdef __cplusplus
}
#endif

#endif // PS_TOKENIZER_H
