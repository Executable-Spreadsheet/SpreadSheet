# Memory Leaks

Here is a quick reference table of each object, their constructor, and their destructor. It may be incomplete; add to it if so.

## Allocator (maybe?)
StackAllocatorCreate(Allocator a, u64 minsize);
StackAllocatorDestroy(Allocator* a);

## TokenList
TokenList* CreateTokenList(Allocator allocator);
void DestroyTokenList(TokenList** tokens);

## AST
AST BuildASTFromTokens(TokenList* tokens, Allocator allocator);
AST ast = {.mem = Allocator allocator}; // no dedicated make fn
ASTFree(AST* tree); 
