#include <libparasheet/lib_internal.h>
#include <stdint.h>
#include <string.h>
#include <util/util.h>

static void ResizeNodes(AST* tree) {
    if (tree->size + 1 <= tree->cap) return;
    u32 oldsize = tree->cap;
    tree->cap = tree->cap ? tree->cap * 2 : 2;
    tree->nodes = Realloc(
        tree->mem,
        tree->nodes,
        oldsize * sizeof(ASTNode),
        tree->cap * sizeof(ASTNode)
    );

    //initialize new memory to zero
    memset(&tree->nodes[oldsize], 0, oldsize);
}

u32 ASTPush(AST* tree) {
    ResizeNodes(tree);
    return tree->size++;
}

SString nodeops[] = {
    sstring("Invalid"), //Invalid
    sstring("INT"),     //INT
    sstring("FLOAT"),   //FLOAT
    sstring("ADD"),     //ADD 
    sstring("SUB"),     //SUB
    sstring("MUL"),     //MUL
    sstring("DIV"),     //DIV

    sstring("CALL"),    //CALL
};


static void ASTPrintNode(FILE* fd, AST* tree, ASTNode* node, u32 indent) {
    for (u32 i = 0; i < indent; i++) {
        print(fd, "|");

        if (i + 1 >= indent) print(fd, "--");
        else print(fd, "  ");
    }
    print(fd, "%s\n", nodeops[node->op]);

    switch(node->op) {
        case AST_INT:
        case AST_FLOAT:
            break;

        case AST_ADD:
        case AST_SUB:
        case AST_MUL:
        case AST_DIV:
        {
            ASTPrintNode(fd, tree, &tree->nodes[node->lchild], indent + 1); 
            ASTPrintNode(fd, tree, &tree->nodes[node->rchild], indent + 1); 
        } break;
        case AST_CALL:
        {
            todo();
        } break;

        case AST_INVALID:
        default: panic();
    }

}

void ASTPrint(FILE* fd, AST* tree) {
    ASTPrintNode(fd, tree, &tree->nodes[tree->size - 1], 0);

}

void ASTFree(AST* tree) {
    Free(tree->mem, tree->nodes, tree->cap * sizeof(ASTNode));
}
