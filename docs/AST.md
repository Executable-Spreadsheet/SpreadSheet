# IMPORTANT!!!

When writing your parser, please build the tree in
Post Order as best as you can. Also be sure to make the root
node the last node you insert. The Print Function assumes
the root is the last node allocated and the plan for future
type inference is to iterate from beginning to end, and always
have the children precede the parents.



```c
u32 ASTPush(AST* tree);
```

This function will allocate a new AST Node and return
back a handle to access it.

```c
#define ASTGet(tree, idx)
```

This function takes in the tree and the u32 handle for
an ASTNode and will return the ASTNode. This allows for
editing the Node via `ASTGet(tree, idx).op`.

It essentially does an index into the AST memory for the
Node.

```c
void ASTPrint(FILE* fd, AST* tree);
```

This Function will format and print an AST tree to a
file.

```c
void ASTFree(AST* tree);
```

This function will free all the memory associated with an
AST.

## Usage

All the Different Node types can be found in:

```c
typedef enum ASTNodeType : u32;
```

When doing type inference, use the types in:

```c
typedef enum ASTValueType : u32;
```

When an ASTNode has to reference some other
persistant information use the `data` member
on the ASTNode.

To set the children of the ASTNode use
the `lchild` and `rchild` nodes. 


## Internal

```c
static void ResizeNodes(AST* tree);
```

This is an internal function used to allocate new AST
Nodes.

```c
static void ASTPrintNode(FILE* fd, AST* tree, ASTNode* node, u32 indent);
```
This is a recursive function used by ASTPrint to format
and print an AST.

```c
SString nodeops[];
```

This is a table of strings corresponding to each Node Operation.
It is used for converting Node types to Strings for printing.

