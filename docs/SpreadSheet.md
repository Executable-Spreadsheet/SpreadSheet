These functions are regarding the internal spreadsheet
representation.

```c
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
```


Notable features are the key and value arrays which are used
for hashing.

The block pool is an array of Blocks. A Block is defined as such.

```c
typedef struct Block {
    u32 nonempty;
    CellValue cells[BLOCK_SIZE * BLOCK_SIZE];
} Block;
```



Here are all the functions for the internal SpreadSheet Data type and their intended usage.

```c
void SpreadSheetSetCell(SpreadSheet* sheet, v2u pos, CellValue value);
CellValue* SpreadSheetGetCell(SpreadSheet* sheet, v2u pos);

void SpreadSheetClearCell(SpreadSheet* sheet, v2u pos);
void SpreadSheetFree(SpreadSheet* sheet);
```

These are the main functions. 

Set Cell is used for writing
a value into a cell that may or may not exist. If the cell
doesn't exist it will be created and written to. 

Get Cell is used to read a cell if it exists. If the cell
doesn't exist the pointer returned will be NULL. This is
useful for both reading and modifying cells. Please DO NOT
USE this for marking cells as empty.

Clear Cell is used to make cells empty and free the block
of cells if possible. Please use this function or Set Cell
to make cells Empty rather than the Get Cell function.

`SpreadSheetFree` This function frees the entire spreadsheet
data structure. However it doesn't free the top level spreadsheet
structure, it is assumed that the caller will be managing that
memory.


```c
u32 SheetBlockInsert(SpreadSheet* sheet, v2u pos, u32 bid);
u32 SheetBlockGet(SpreadSheet* sheet, v2u pos);
void SheetBlockDelete(SpreadSheet* sheet, v2u pos);
```

These functions are for efficient iteration and interaction.

Block Insert. This function will get or create a block of cells.
The bid parameter should be set to UINT32_MAX unless you are
attempting to insert a specific block at a given position. This
parameter should always be UINT32_MAX. This function returns
an index into `sheet->blockpool` which can be used to obtain a
pointer to a Block.

Block Get. This function will return UINT32_MAX if the block
does not exist. Otherwise it is identical to Block Insert
except it does not have a bid parameter. Prefer this for
iteration and editing.

Block Delete. This function will clear a block and mark it
for reuse. This will essentially free a block of cells.


## Internal Functions

```c
static void AllocBlock(SpreadSheet* sheet);
static u32 PickBlock(SpreadSheet* sheet);
static void FreeBlock(SpreadSheet* sheet, u32 blockid);
static void ResizeSheet(SpreadSheet* sheet);
```


These four functions are used to encapsulate functionality which
may be used multiple times or would be overly indented and
reduce readability.

Alloc Block is used for increasing the size of the Block Pool
when more blocks are required.

Pick Block is used to pick a free block which is not being
used when a new block is required.

Free Block is used to mark a block free and to clear
a Block back to zero after it is no longer used.

Resize Sheet is used to resize the hash map when the load
factor drops below the constant `MAX_LOAD_FACTOR`. It essentially
duplicates the key and value arrays and then uses Block Insert
to reinsert each nonempty key slot back into the new, larger map.
This should never be called from anywhere other than Block Insert.

One last piece of this is the `Invalid` constant. This is
used as a reserved invalid key for the hash map. It is
used as a way to test if a hashmap slot is empty or not,
and is used in conjunction with CMPV2.
