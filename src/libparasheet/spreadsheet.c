#include <libparasheet/lib_internal.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>
#include <util/util.h>

/*
+---------------------------------------------------+
|   INFO(ELI):                                      |
|   This file should contain the implementation for |
|   the internal spreadsheet representation.        |
|                                                   |
|   *It may be broken out into other files later.*  |
+---------------------------------------------------+
*/

const static v2u Invalid = {UINT32_MAX, UINT32_MAX};

static void AllocBlock(SpreadSheet* sheet) {
	u32 oldsize = sheet->bcap;

	while (sheet->bsize + 1 >= sheet->bcap) {
		sheet->bcap = sheet->bcap ? sheet->bcap * 2 : 2;
	}

	sheet->blockpool =
		Realloc(sheet->mem, sheet->blockpool, oldsize * sizeof(Block),
				sheet->bcap * sizeof(Block));
	sheet->freestatus =
		Realloc(sheet->mem, sheet->freestatus, oldsize * sizeof(i32),
				sheet->bcap * sizeof(i32));

	// add new blocks to free list
	for (u32 i = oldsize; i < sheet->bcap; i++) {
		sheet->freestatus[sheet->fsize++] = i;
	}

	memset(&sheet->blockpool[oldsize], 0,
		   (sheet->bcap - oldsize) * sizeof(Block));
}

static u32 PickBlock(SpreadSheet* sheet) {
	if (!sheet->fsize)
		AllocBlock(sheet);
	return sheet->freestatus[--sheet->fsize];
}

static void FreeBlock(SpreadSheet* sheet, u32 blockid) {
	sheet->freestatus[sheet->fsize++] = blockid;
	memset(&sheet->blockpool[blockid], 0, sizeof(Block));
}

static void ResizeSheet(SpreadSheet* sheet) {
	if (sheet->size + 1 < (sheet->cap * MAX_LOAD_FACTOR))
		return;

	u32 oldsize = sheet->cap;

	while (sheet->size + 1 >= (sheet->cap * MAX_LOAD_FACTOR)) {
		sheet->cap = sheet->cap ? sheet->cap * 2 : 4;
	}

	u32* oldvalues = sheet->values;
	v2u* oldkeys = sheet->keys;

	sheet->values = Alloc(sheet->mem, sheet->cap * sizeof(u32));
	sheet->keys = Alloc(sheet->mem, sheet->cap * sizeof(v2u));
	sheet->size = 0;

	for (u32 i = 0; i < sheet->cap; i++) {
		sheet->keys[i] = Invalid;
	}

	// TODO(ELI): reinsert items
	for (u32 i = 0; i < oldsize; i++) {
		// log("\tkey: (%d %d)", oldkeys[i].x, oldkeys[i].y);
		if (!CMPV2(oldkeys[i], Invalid)) {
			log("reinsert: %d", oldvalues[i]);
			SheetBlockInsert(sheet, oldkeys[i], oldvalues[i]);
		}
	}

	Free(sheet->mem, oldvalues, oldsize * sizeof(u32));
	Free(sheet->mem, oldkeys, oldsize * sizeof(v2u));
}

u32 SheetBlockInsert(SpreadSheet* sheet, v2u pos, u32 bid) {
	ResizeSheet(sheet);

	// maybe in future we just use power of 2 sizes?
	u32 idx = hash((u8*)&pos, sizeof(pos)) % sheet->cap;

	// search
	for (u32 i = 0; i < sheet->cap; i++) {
		v2u curr = sheet->keys[idx];
		if (CMPV2(curr, pos)) {
			return sheet->values[idx];
		}

		if (CMPV2(curr, Invalid)) {
			break;
		}
		idx = (idx + 1) % sheet->cap; // using linear probing for now
	}

	sheet->keys[idx] = pos;

	if (bid != UINT32_MAX)
		sheet->values[idx] = bid;
	else {
		sheet->values[idx] = PickBlock(sheet);
	}

	sheet->size++;
	return sheet->values[idx];
}

u32 SheetBlockGet(SpreadSheet* sheet, v2u pos) {
    if (!sheet->cap) {
        //If there are no mappings
        //return -1
        return -1;
    }


    u32 idx = hash((u8*)&pos, sizeof(pos)) % sheet->cap;

	for (u32 i = 0; i < sheet->cap; i++) {
		v2u curr = sheet->keys[idx];
		if (CMPV2(curr, pos)) {
			return sheet->values[idx];
		}

		if (CMPV2(curr, Invalid)) {
			return -1;
		}
		idx = (idx + 1) % sheet->cap; // using linear probing for now
	}

	panic();
}

void SheetBlockDelete(SpreadSheet* sheet, v2u pos) {
	u32 idx = hash((u8*)&pos, sizeof(pos)) % sheet->cap;

	for (u32 i = 0; i < sheet->size; i++) {
		v2u curr = sheet->keys[idx];
		if (CMPV2(curr, pos)) {
			FreeBlock(sheet, sheet->values[idx]);
			sheet->keys[idx] = Invalid;
			sheet->size--;
			return;
		}

		if (CMPV2(curr, Invalid)) {
			return;
		}
		idx = (idx + 1) % sheet->cap; // using linear probing for now
	}

	panic();
}

void SpreadSheetSetCell(SpreadSheet* sheet, v2u pos, CellValue val) {
	v2u blockpos = CELL_TO_BLOCK(pos);
	u32 blockid = SheetBlockInsert(sheet, blockpos, UINT32_MAX);
	Block* block = &sheet->blockpool[blockid];

	v2u offset = CELL_TO_OFFSET(pos);
	u32 index = offset.x + offset.y * BLOCK_SIZE;

	// NOTE(ELI): Block Insert forces the block to exist if it doesn't already
	// So this is always safe
	if (val.t == CT_EMPTY) {
		if (block->cells[index].t != CT_EMPTY)
			block->nonempty--;
		if (block->nonempty <= 0)
			SheetBlockDelete(sheet, blockpos);
		return;
	} else if (block->cells[index].t == CT_EMPTY)
		block->nonempty++;
	block->cells[index] = val;
}

// NOTE(ELI): This can be NULL because the Cell
// may not exist. This was better than the alternative
// which would mean that calling this function could
// actually cause memory allocations which is super
// unintuitive.
CellValue* SpreadSheetGetCell(SpreadSheet* sheet, v2u pos) {
	v2u blockpos = CELL_TO_BLOCK(pos);
	u32 blockid = SheetBlockGet(sheet, blockpos);

	if (blockid == UINT32_MAX) {
		return NULL;
	}

	Block* block = &sheet->blockpool[blockid];
	v2u offset = CELL_TO_OFFSET(pos);
	u32 index = offset.x + offset.y * BLOCK_SIZE;
	return &block->cells[index];
}

void SpreadSheetClearCell(SpreadSheet* sheet, v2u pos) {
	v2u blockpos = CELL_TO_BLOCK(pos);
	u32 blockid = SheetBlockInsert(sheet, blockpos, UINT32_MAX);
	Block* block = &sheet->blockpool[blockid];

	// ensure block exists
	if (!block) {
		return;
	}

	v2u offset = CELL_TO_OFFSET(pos);
	u32 index = offset.x + offset.y * BLOCK_SIZE;

	if (block->cells[index].t != CT_EMPTY) {
		block->nonempty--;
	}

	if (block->nonempty > 0)
		return;
	// if the block is completely empty

	SheetBlockDelete(sheet, blockpos);
}

void SpreadSheetFree(SpreadSheet* sheet) {
	Free(sheet->mem, sheet->blockpool, sheet->bcap * sizeof(Block));
	Free(sheet->mem, sheet->freestatus, sheet->bcap * sizeof(u32));
	Free(sheet->mem, sheet->keys, sheet->cap * sizeof(v2u));
	Free(sheet->mem, sheet->values, sheet->cap * sizeof(u32));
}
