#ifndef PS_INTERNAL_H
#define PS_INTERNAL_H
#include <util/util.h>

/*
+---------------------------------------------------+
|   INFO(ELI):                                      |
|   This is the main internal header file. By       |
|   default almost everything should go in here     |
|   that shouldn't be exposed to the users of the   |
|   library. We should be fine to just have one     |
|   file unless it gets way too big and everyone    |
|   decides to split it up.                         |
+---------------------------------------------------+
*/


#define BLOCK_SIZE 16
#define MAX_LOAD_FACTOR 0.6

#define CELL_TO_BLOCK(c) \
    ((v2u){c.x / BLOCK_SIZE, c.y / BLOCK_SIZE})

#define CELL_TO_OFFSET(c) \
    ((v2u){c.x % BLOCK_SIZE, c.y % BLOCK_SIZE})


//Gonna use a tagged union for spreadsheet values.
//It just makes a lot of sense and it is fairly compact.

typedef enum CellType : u32 {
    CT_EMPTY = 0,
    CT_CODE,
    CT_TEXT,
    CT_INT,
    CT_FLOAT,
} CellType;

typedef struct CellValue {
    CellType t;
    union {
        u32 i;
        f32 f;
        u32 index; //index into external buffer
    } d;
} CellValue;


typedef struct Block {
    u32 nonempty; //keeps track of nonempty cells,
                  //when empty it gets marked as free

    CellValue cells[BLOCK_SIZE * BLOCK_SIZE];
} Block;

//TODO(ELI): In future organize to minimize padding
//rn things are split based on usage but this should be
//improved in the future.
typedef struct SpreadSheet {
    Allocator mem; //probably should be global allocator but
                // might as well give ourselves options
    //main cell map
    u32* values;
    v2u* keys;
    u32 size;

    //pool of reusable blocks
    Block* blockpool;
    u32 bsize;

    i32* freestatus;
    u32 fsize;

    u32 bcap;
    u32 cap;
} SpreadSheet;


void SpreadSheetSetCell(SpreadSheet* sheet, v2u pos, CellValue value);
CellValue* SpreadSheetGetCell(SpreadSheet* sheet, v2u pos);

void SpreadSheetClearCell(SpreadSheet* sheet, v2u pos);
void SpreadSheetFree(SpreadSheet* sheet);



//INFO(ELI): I decided to have these return indicies
//since indicies are mostly stable and remain
//valid even after a resize.
//
//They also are required for the SpreadSheet to
//reuse empty blocks.
u32 SheetBlockInsert(SpreadSheet* sheet, v2u pos, u32 bid);
u32 SheetBlockGet(SpreadSheet* sheet, v2u pos);
void SheetBlockDelete(SpreadSheet* sheet, v2u pos);

/*
+-------------------------------------------------------------------+
|   INFO(ELI): Here is the AST implementation. If everyone agrees   |
|   later we can break this out into its own header, but for        |
|   the moment it makes sense to just throw everything in here      |
|   since this header is fairly small.                              |
+-------------------------------------------------------------------+
*/

typedef enum ASTNodeType : u32 {
    AST_INVALID = 0, //mark unintialized Node as invalid

    //literals
    AST_INT,
    AST_FLOAT,

    //Ops
    AST_ADD,
    AST_SUB,
    AST_MUL,
    AST_DIV,

    //Probably want this
    AST_CALL,
} ASTNodeOp;

typedef enum ASTValueType : u32 {
    V_INT,
    V_FLOAT,
} ASTValueType;

/*
INFO(ELI): So basically I decided to throw
child indicies into the ASTNode since it allows
for traditional pre order and inorder traversals
as well as the more useful post order traversal.

This might get changed later since it can't support
more than 2 children, or if we do it is a constant
memory cost.

*/
typedef struct ASTNode {
    ASTNodeOp op;
    ASTValueType vt;

    //Union for storing literal values
    union {
        u32 i; //Symbols use this as an index
        f32 f;
    } data;

    u32 lchild;
    u32 rchild;
} ASTNode;

typedef struct AST {
    Allocator mem;
    ASTNode* nodes;
    u32 size;
    u32 cap;
} AST;

#define ASTGet(tree, idx) \
    ((tree)->nodes[idx]) 

u32 ASTPush(AST* tree);

void ASTPrint(FILE* fd, AST* tree);
void ASTFree(AST* tree);


#endif
