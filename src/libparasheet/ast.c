#include <libparasheet/lib_internal.h>
#include <stdint.h>
#include <string.h>
#include <util/util.h>

static void ResizeNodes(AST* tree) {
	if (tree->size + 1 <= tree->cap)
		return;
	u32 oldsize = tree->cap;
	tree->cap = tree->cap ? tree->cap * 2 : 2;
	tree->nodes = Realloc(tree->mem, tree->nodes, oldsize * sizeof(ASTNode),
						  tree->cap * sizeof(ASTNode));

	// initialize new memory to zero
	memset(&tree->nodes[oldsize], 0, oldsize);
}

u32 ASTPush(AST* tree) {
	ResizeNodes(tree);
	return tree->size++;
}

u32 ASTCreateNode(AST* tree, ASTNodeOp op, u32 lchild, u32 mchild, u32 rchild) {
	u32 new_node_index = ASTPush(tree);
	tree->nodes[new_node_index].op = op;

	tree->nodes[new_node_index].lchild = lchild;
	tree->nodes[new_node_index].mchild = mchild;
	tree->nodes[new_node_index].rchild = rchild;

	return new_node_index;
}

SString nodeops[] = {
	sstring("Invalid"),

	// Literals
	sstring("int"),
	sstring("float"),

	// Types
	sstring("INT"),
	sstring("FLOAT"),

	// Variables
	sstring("ID"),
	sstring("LET"),
	sstring("[x,y]"),
	sstring("x=y"),

	// Ops
	sstring("+"),
	sstring("-"),
	sstring("*"),
	sstring("/"),
	sstring("FLOAT -> INT"),
	sstring("INT -> FLOAT"),
	sstring("#"),
	sstring(":"),

	// Control Flow
	sstring("SEQ"),
	sstring("IF () {} ELSE {}"),
	sstring("WHILE () {}"),
	sstring("FOR (,) {}"),
	sstring("RETURN ()"),
    sstring("Scope Pop"),
    sstring("Scope Push"),


	// Function Things
	sstring("=(...)"),
	sstring("(...)"),
	sstring("ID(...)"),
	sstring("(...)"),
};

static void ASTPrintNode(FILE* fd, AST* tree, ASTNode* node, u32 indent) {
	for (u32 i = 0; i < indent; i++) {
		print(fd, "|");

		if (i + 1 >= indent)
			print(fd, "--");
		else
			print(fd, "  ");
	}
	if (node->op == AST_INVALID) {
		err("AST node operation invalid!");
		panic();
	}
	print(fd, "%s (%d)\n", nodeops[node->op], node->data.i);

	if (node->lchild != UINT32_MAX) {
		ASTPrintNode(fd, tree, &tree->nodes[node->lchild], indent + 1);
	}
	if (node->mchild != UINT32_MAX) {
		ASTPrintNode(fd, tree, &tree->nodes[node->mchild], indent + 1);
	}
	if (node->rchild != UINT32_MAX) {
		ASTPrintNode(fd, tree, &tree->nodes[node->rchild], indent + 1);
	}
}

void ASTPrint(FILE* fd, AST* tree) {
	ASTPrintNode(fd, tree, &tree->nodes[tree->size - 1], 0);
}

void ASTFree(AST* tree) {
	Free(tree->mem, tree->nodes, tree->cap * sizeof(ASTNode));
}
