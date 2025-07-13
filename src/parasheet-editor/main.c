#include <stdint.h>
#include <stdio.h>
#include <util/util.h>
#include <libparasheet/lib_internal.h>

int main() {

    AST a = {
        .mem = GlobalAllocatorCreate(),
    };

    {
        ASTNode n = {
            .op = AST_INT,
            .lchild = UINT32_MAX,
            .rchild = UINT32_MAX,
        };
        ASTInsert(&a, n);
    }
    {
        ASTNode* n = ASTPush(&a);
        n->op = AST_INT;
    }
    {
        ASTNode* n = ASTPush(&a);
        n->op = AST_ADD;
        n->lchild = 0;
        n->rchild = 1;
    }

    {
        ASTNode n = {
            .op = AST_INT,
            .lchild = UINT32_MAX,
            .rchild = UINT32_MAX,
        };
        ASTInsert(&a, n);
    }
    {
        ASTNode* n = ASTPush(&a);
        n->op = AST_INT;
    }
    {
        ASTNode* n = ASTPush(&a);
        n->op = AST_ADD;
        n->lchild = 3;
        n->rchild = 4;
    }
    {
        ASTNode* n = ASTPush(&a);
        n->op = AST_ADD;
        n->lchild = 2;
        n->rchild = 5;
    }

    ASTPrint(stdout, &a);

}

