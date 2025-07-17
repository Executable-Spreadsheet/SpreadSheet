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

## AST Node Types
```c
AST_INVALID
```
This indicates that there was some issue in creating the AST, and this node is invalid

I'll finish this later my laptop is dying

```c
AST_INT_LITERAL,
```

```c
AST_FLOAT_LITERAL,
```

```c
AST_INT_TYPE,
```

```c
AST_FLOAT_TYPE,
```

```c
AST_ID,
```

```c
AST_DECLARE_VARIABLE,
```

```c
AST_GET_CELL_REF,
```

```c
AST_ASSIGN_VALUE,
```

```c
AST_ADD
```

```c
AST_SUB
```

```c
AST_MUL
```

```c
AST_DIV
```

```c
AST_FLOAT_TO_INT
```

```c
AST_INT_TO_FLOAT
```

```c
AST_COORD_TRANSFORM
```

```c
AST_RANGE
```

```c
AST_SEQ
```

```c
AST_IF_ELSE
```

```c
AST_WHILE
```
```c
AST_FOR
```

```c
AST_RETURN
```

```c
AST_HEADER
```

```c
AST_HEADER_ARGS
```

```c
AST_CALL
```

```c
AST_FUNC_ARGS
```
