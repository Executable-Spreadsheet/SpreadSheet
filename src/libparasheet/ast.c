#include <libparasheet/lib_internal.h>
#include <util/util.h>


void ASTInsert(AST* tree, ASTNode node) {
    if (tree->size + 1 > tree->cap) {
        u32 oldsize = tree->cap;
        tree->cap = tree->cap ? tree->cap * 2 : 2;
        tree->nodes = Realloc(
                        tree->mem,
                        tree->nodes,
                        oldsize * sizeof(ASTNode),
                        tree->cap * sizeof(ASTNode)
        );
    }

    tree->nodes[tree->size++] = node;
}

SString ASTFormat(Allocator mem, AST* tree) {
    todo();
}

void ASTFree(AST* tree) {
    Free(tree->mem, tree->nodes, tree->cap * sizeof(ASTNode));
}
