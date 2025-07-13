#include <libparasheet/lib_internal.h>
#include <stdint.h>
#include <stdio.h>
#include <util/util.h>

int main() {

	AST tree = {
		.mem = GlobalAllocatorCreate(),
	};

	u32 a1, a2;
	{

		u32 l = ASTPush(&tree);
		ASTGet(&tree, l) = (ASTNode){.op = AST_INT};

		u32 r = ASTPush(&tree);
		ASTGet(&tree, r) = (ASTNode){.op = AST_INT};

		a1 = ASTPush(&tree);
		ASTGet(&tree, a1) = (ASTNode){.op = AST_ADD, .lchild = l, .rchild = r};
	}
	{

		u32 l = ASTPush(&tree);
		ASTGet(&tree, l) = (ASTNode){.op = AST_INT};

		u32 r = ASTPush(&tree);
		ASTGet(&tree, r) = (ASTNode){.op = AST_INT};

		a2 = ASTPush(&tree);
		ASTGet(&tree, a2) = (ASTNode){.op = AST_ADD, .lchild = l, .rchild = r};
	}
	u32 p = ASTPush(&tree);
	ASTGet(&tree, p) = (ASTNode){.op = AST_ADD, .lchild = a1, .rchild = a2};

	ASTPrint(stdout, &tree);
}
