#include <libparasheet/lib_internal.h>
#include <stdint.h>
#include <stdio.h>
#include <util/util.h>

#define EPS UINT32_MAX

int main() {

	AST tree = {
		.mem = GlobalAllocatorCreate(),
	};

	u32 a1, a2;
	{
		u32 l = ASTCreateNode(&tree, AST_INT_LITERAL, EPS, EPS, EPS);
		u32 m = ASTCreateNode(&tree, AST_INT_LITERAL, EPS, EPS, EPS);
		u32 r = ASTCreateNode(&tree, AST_INT_LITERAL, EPS, EPS, EPS);
		a1 = ASTCreateNode(&tree, AST_DIV, l, m, r);
	}
	{
		u32 l = ASTCreateNode(&tree, AST_INT_LITERAL, EPS, EPS, EPS);
		u32 r = ASTCreateNode(&tree, AST_INT_LITERAL, EPS, EPS, EPS);
		a2 = ASTCreateNode(&tree, AST_SUB, l, r, EPS);
	}
	u32 p = ASTCreateNode(&tree, AST_ADD, a1, a2, EPS);
	u32 x = ASTCreateNode(&tree, AST_FLOAT_LITERAL, EPS, EPS, EPS);
	u32 y = ASTCreateNode(&tree, AST_FLOAT_LITERAL, EPS, EPS, EPS);
	u32 q = ASTCreateNode(&tree, AST_ADD, p, x, EPS);
	u32 s = ASTCreateNode(&tree, AST_ADD, y, q, y);

	ASTPrint(stdout, &tree);
	log("size: %d", sizeof(tree));

    ASTFree(&tree);
}
