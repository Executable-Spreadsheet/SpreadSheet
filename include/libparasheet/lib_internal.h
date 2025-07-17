#ifndef PS_INTERNAL_H
#define PS_INTERNAL_H
#include <util/util.h>
#include <libparasheet/tokenizer_types.h>

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

/*
+--------------------------------------+
|   INFO(ELI): String Interning        |
+--------------------------------------+
*/

typedef struct StringTable {
    Allocator mem; //should almost certainly be Global allocator

    //Hash table
    u32* meta; //metadata used for Robin Hood Hashing
    u32* vals; //indicies into string buffer (contains StrIDs)
    u32 size;
    u32 cap;

    //String Storage
    SString* strings;
    u32* entry; //back reference to hashtable
    u32* freelist; //free slots in string buffer
    u32 fsize;
    u32 ssize;
    u32 scap;


} StringTable;

typedef u32 StrID;

StrID StringAdd(StringTable* table, i8* string);
StrID StringAddS(StringTable* table, SString string);

//NOTE(ELI): If you allocated the string in the table
//you have to free the returned string manually. The table
//allows for things like string literals and so can't
//free the strings automatically
SString StringDel(StringTable* table, StrID index);
const SString StringGet(StringTable* table, StrID index);

#define StringCmp(a, b) \
    (a == b)

void StringFree(StringTable* table);




/*
+--------------------------------------+
|   INFO(ELI): Spread Sheet Section    |
+--------------------------------------+
*/

#define BLOCK_SIZE 16
#define MAX_LOAD_FACTOR 0.6

#define CELL_TO_BLOCK(c) ((v2u){c.x / BLOCK_SIZE, c.y / BLOCK_SIZE})

#define CELL_TO_OFFSET(c) ((v2u){c.x % BLOCK_SIZE, c.y % BLOCK_SIZE})

#define CELL_TO_INDEX(v) \
    (v.y + v.x * BLOCK_SIZE)

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
		u32 index; // index into external buffer
	} d;
} CellValue;

typedef struct Block {
	u32 nonempty; // keeps track of nonempty cells,
				  // when empty it gets marked as free

	CellValue cells[BLOCK_SIZE * BLOCK_SIZE];
} Block;

// TODO(ELI): In future organize to minimize padding
// rn things are split based on usage but this should be
// improved in the future.
typedef struct SpreadSheet {
	Allocator mem; // probably should be global allocator but
				   //  might as well give ourselves options
	// main cell map
	u32* values;
	v2u* keys;
	u32 size;
    u32 tomb;

	// pool of reusable blocks
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

// INFO(ELI): I decided to have these return indicies
// since indicies are mostly stable and remain
// valid even after a resize.
//
// They also are required for the SpreadSheet to
// reuse empty blocks.
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
	AST_INVALID = 0, // mark unintialized Node as invalid

	// Literals
	AST_INT_LITERAL,
	AST_FLOAT_LITERAL,

	// Types
	AST_INT_TYPE,
	AST_FLOAT_TYPE,

	// Variables
	AST_ID,
	AST_DECLARE_VARIABLE,
	AST_GET_CELL_REF,
	AST_ASSIGN_VALUE,

	// Ops
	AST_ADD, // +
	AST_SUB, // -
	AST_MUL, // *
	AST_DIV, // /
	AST_FLOAT_TO_INT, // implicit
	AST_INT_TO_FLOAT, // implicit
	AST_COORD_TRANSFORM, // #
	AST_RANGE, // :

	// Control Flow
	AST_SEQ,
	AST_IF_ELSE,
	AST_WHILE,
	AST_FOR,
	AST_RETURN,

	// Function Things
	AST_HEADER,
	AST_HEADER_ARGS,
	AST_CALL,
	AST_FUNC_ARGS,
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

	// Union for storing literal values
	union {
		u32 i; // Symbols use this as an index
		f32 f;
	} data;

	u32 lchild;
	u32 mchild;
	u32 rchild;
} ASTNode;

typedef struct AST {
	Allocator mem;
	ASTNode* nodes;
	u32 size;
	u32 cap;
} AST;

#define ASTGet(tree, idx) ((tree)->nodes[idx])

u32 ASTPush(AST* tree);
u32 ASTCreateNode(AST* tree, ASTNodeOp op, u32 lchild, u32 mchild, u32 rchild);

void ASTPrint(FILE* fd, AST* tree);
void ASTFree(AST* tree);

AST* BuildASTFromTokens(TokenList* tokens, Allocator allocator);

/*
+-----------------------------------------------+
|   INFO(ELI):                                  |
|                                               |
|   Here is the section for the Symbol table.   |
+-----------------------------------------------+
*/


typedef enum SymbolType : u32 {
    S_INVALID = 0,
    S_VAR,
} SymbolType;

typedef struct SymbolEntry {
    SymbolType type; 
    u32 idx;
} SymbolEntry;

//Insertion and getting only, never delete
typedef struct SymbolMap {
    Allocator mem;

    u32* keys;
    SymbolEntry* entries;
    u32 size;
    u32 cap;
} SymbolMap;

//deletions happen via popping a scope
typedef struct SymbolTable {
    Allocator mem;
    SymbolMap* scopes;
    u32 size;
    u32 cap;
} SymbolTable;

u32 SymbolMapInsert(SymbolMap* map, StrID key);
u32 SymbolMapGet(SymbolMap* map, StrID key);
void SymbolMapFree(SymbolMap* map);


void SymbolInsert(SymbolTable* table, StrID key, SymbolEntry entry);
SymbolEntry SymbolGet(SymbolTable* table, StrID key);

void SymbolPushScope(SymbolTable* table);
void SymbolPopScope(SymbolTable* table);


#endif
