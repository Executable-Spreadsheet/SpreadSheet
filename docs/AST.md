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
Note: In the section below, take `EPS` to mean `UINT32_MAX`.

```c
AST_INVALID
```
This indicates that there was some issue in creating the AST, and this node is invalid

```c
AST_INT_LITERAL,
```
This indicates an int literal. The data field of the struct should be populated with the int. When this is set, `lchild`, `mchild`, and `rchild` should all be set to `EPS`.

```c
AST_FLOAT_LITERAL,
```
This indicates a float literal. The data field of the struct should be populated with the float. When this is set, `lchild`, `mchild`, and `rchild` should all be set to `EPS`.

```c
AST_INT_TYPE,
```
This indicates the int type, as used in variable declaration. When this is set, `lchild`, `mchild`, and `rchild` should all be set to `EPS`.

```c
AST_FLOAT_TYPE,
```
This indicates the float type, as used in variable declaration. When this is set, `lchild`, `mchild`, and `rchild` should all be set to `EPS`.

```c
AST_ID,
```
This indicates a user provided identifier, i.e. a variable. When this is set, `lchild`, `mchild`, and `rchild` should all be set to `EPS`.

```c
AST_DECLARE_VARIABLE,
```
This indicates a variable declaration. When this is set, `lchild` should be the identifier, `mchild` should be the type, and `rchild` should be the value.

```c
AST_GET_CELL_REF,
```
This indicates getting a reference to a cell `[x,y]`. When this is set, `lchild` should be `x`, `mchild` should be `y`, and `rchild` should be `EPS`.

```c
AST_ASSIGN_VALUE,
```
This indicates assigning a value to a variable. When this is set, `lchild` should be the variable's identifier, `mchild` should be the value to set the variable to, and `rchild` should be `EPS`.

```c
AST_ADD
```
This indicates getting a sum `x + y`. When this is set, `lchild` should be `x`, `mchild` should be `y`, and `rchild` should be `EPS`.

```c
AST_SUB
```
This indicates getting a difference `x - y`. When this is set, `lchild` should be `x`, `mchild` should be `y`, and `rchild` should be `EPS`.

```c
AST_MUL
```
This indicates getting a product `x * y`. When this is set, `lchild` should be `x`, `mchild` should be `y`, and `rchild` should be `EPS`.

```c
AST_DIV
```
This indicates getting a quotient `x / y`. When this is set, `lchild` should be `x`, `mchild` should be `y`, and `rchild` should be `EPS`.

```c
AST_FLOAT_TO_INT
```
This indicates converting a float to an int. `lchild` is the number to be converted, while `mchild` and `rchild` should be set to `EPS`.

```c
AST_INT_TO_FLOAT
```
This indicates converting an to a float. `lchild` is the number to be converted, while `mchild` and `rchild` should be set to `EPS`.

```c
AST_COORD_TRANSFORM
```
This indicates a conversion to global coordinates. `lchild` is the number to be converted, while `mchild` and `rchild` should be set to `EPS`.

```c
AST_RANGE
```
This generates an iterator from `lchild` to `mchild`. `rchild` should be set to `EPS`.

```c
AST_SEQ
```
This indicates two statements to be run sequentially. `lchild` first, then `mchild`. `rchild` should be set to `EPS`.

```c
AST_IF_ELSE
```
This indicates an if-else statement. `lchild` should contain the condition, `mchild` should contain the if body, and `rchild` should either be `EPS` or the else body, depending on whether the code contains an else block.

```c
AST_WHILE
```
This indicates a while loop. `lchild` should contain the condition, and `mchild` should contain the body. `rchild` should be set to `EPS`

```c
AST_FOR
```
This indicates a for loop. `lchild` should be an identifier for an iterator variable, `mchild` should be an iterator (created with the : operator), and `rchild` should be the body of the for loop.

```c
AST_RETURN
```
This indicates a return statement. `lchild` should contain the return value, while `mchild` and `rchild` should be set to `EPS`.

```c
AST_HEADER
```
This indicates that a cell is a function callable by other cells. `lchild` should contain the function arguments or be set to `EPS` if there are none. `mchild` should contain the function body. `rchild` should be set to EPS.

```c
AST_HEADER_ARGS
```
This contains an argument in a function header. `lchild` is the identifier of the argument, `mchild` contains the type of the argument, and `rchild` points to more arguments or to `EPS` if there are none.

```c
AST_CALL
```
This indicates calling a function. `lchild` contains the function, and `mchild` contains the arguments, or `EPS` if there are none.

```c
AST_FUNC_ARGS
```
This contains an argument used when calling a function. `lchild` contains the argument, and `mchild` contains either more arguments, or `EPS` if there are none.
