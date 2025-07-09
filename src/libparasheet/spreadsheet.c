#include <libparasheet/lib_internal.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>
#include <util/util.h>


/*
+---------------------------------------------------+
|   INFO(ELI):                                      |
|   This file should contain the implementation for |
|   the internal spreadsheet representation. It may |
|   be broken out into other files later.           |
+---------------------------------------------------+
*/


const static v2u Invalid = {UINT32_MAX, UINT32_MAX};

static void AllocBlock(SpreadSheet* s) {
    u32 oldsize = s->bcap;

    while (s->bsize + 1 >= s->bcap) {
        s->bcap = s->bcap ? s->bcap * 2 : 2;
    }

    s->blockpool = Realloc(s->mem, s->blockpool, oldsize * sizeof(Block), s->bcap * sizeof(Block)); 
    s->freestatus = Realloc(s->mem, s->freestatus, oldsize * sizeof(i32), s->bcap * sizeof(i32));

    //add new blocks to free list
    for (u32 i = oldsize; i < s->bcap; i++) {
        s->freestatus[s->fsize++] = i;
    }

    memset(&s->blockpool[oldsize], 0, (s->bcap - oldsize) * sizeof(Block));
}

static u32 PickBlock(SpreadSheet* s) {
    if (!s->fsize) AllocBlock(s);
    return s->freestatus[--s->fsize];
}

static void FreeBlock(SpreadSheet* s, u32 i) {
    s->freestatus[s->fsize++] = i;
    memset(&s->blockpool[i], 0, sizeof(Block));
}


static void ResizeSheet(SpreadSheet* s) {
    if (s->size + 1 < (s->cap * MAX_LOAD_FACTOR)) return;

    u32 oldsize = s->cap;

    while (s->size + 1 >= (s->cap * MAX_LOAD_FACTOR)) { 
        s->cap = s->cap ? s->cap * 2 : 4;
    }

    u32* oldvalues = s->values;
    v2u* oldkeys = s->keys;

    s->values = Alloc(s->mem, s->cap * sizeof(u32)); 
    s->keys = Alloc(s->mem, s->cap * sizeof(v2u));
    s->size = 0;

    for (u32 i = 0; i < s->cap; i++) {
        s->keys[i] = Invalid;
    }

    //TODO(ELI): reinsert items 
    for (u32 i = 0; i < oldsize; i++) {
        //log("\tkey: (%d %d)", oldkeys[i].x, oldkeys[i].y);
        if (!CMPV2(oldkeys[i], Invalid)) {
            log("reinsert: %d", oldvalues[i]);
            BlockInsert(s, oldkeys[i], oldvalues[i]);
        }
    }


    Free(s->mem, oldvalues, oldsize * sizeof(u32));
    Free(s->mem, oldkeys, oldsize * sizeof(v2u));
}


u32 BlockInsert(SpreadSheet* sheet, v2u pos, u32 bid) {
    ResizeSheet(sheet);

    //maybe in future we just use power of 2 sizes?
    u32 idx = hash((u8*)&pos, sizeof(pos)) % sheet->cap;

    
    //search
    for (u32 i = 0; i < sheet->cap; i++) {
        v2u curr = sheet->keys[idx];
        if (CMPV2(curr, pos)) {
            return sheet->values[idx];
        }

        if (CMPV2(curr, Invalid)) {
            break;
        }
        idx = (idx + 1) % sheet->cap; //using linear probing for now
    }

    sheet->keys[idx] = pos;

    if (bid != UINT32_MAX) sheet->values[idx] = bid;
    else {
        sheet->values[idx] = PickBlock(sheet); 
    }

    sheet->size++;
    return sheet->values[idx];
}

u32 BlockGet(SpreadSheet* sheet, v2u pos) {
    u32 idx = hash((u8*)&pos, sizeof(pos)) % sheet->cap;

    for (u32 i = 0; i < sheet->cap; i++) {
        v2u curr = sheet->keys[idx];
        if (CMPV2(curr, pos)) {
            return sheet->values[idx];
        }

        if (CMPV2(curr, Invalid)) {
            return -1;
        }
        idx = (idx + 1) % sheet->cap; //using linear probing for now
    }

    panic();
}

void BlockDelete(SpreadSheet* sheet, v2u pos) {
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
        idx = (idx + 1) % sheet->cap; //using linear probing for now
    }

    panic();
}



void SpreadSheetSetCell(SpreadSheet* s, v2u p, CellValue v) {
    v2u blockpos = CELL_TO_BLOCK(p);
    u32 blockid = BlockInsert(s, blockpos, UINT32_MAX);
    Block* block = &s->blockpool[blockid];

    v2u offset = CELL_TO_OFFSET(p);
    u32 index = offset.x + offset.y * BLOCK_SIZE;

    //NOTE(ELI): Block Insert forces the block to exist if it doesn't already
    //So this is always safe
    if (v.t == CT_EMPTY) {
        if (block->cells[index].t != CT_EMPTY) block->nonempty--;
        if (block->nonempty <= 0) BlockDelete(s, blockpos);
        return;
    }
    else if (block->cells[index].t == CT_EMPTY) block->nonempty++;
    block->cells[index] = v;
}

//NOTE(ELI): This can be NULL because the Cell
//may not exist. This was better than the alternative
//which would mean that calling this function could
//actually cause memory allocations which is super
//unintuitive.
CellValue* SpreadSheetGetCell(SpreadSheet* s, v2u p) {
    v2u blockpos = CELL_TO_BLOCK(p);
    u32 blockid = BlockGet(s, blockpos);

    if (blockid == UINT32_MAX) {
        return NULL;
    }

    Block* block = &s->blockpool[blockid];
    v2u offset = CELL_TO_OFFSET(p);
    u32 index = offset.x + offset.y * BLOCK_SIZE;
    return &block->cells[index];
}

void SpreadSheetClearCell(SpreadSheet* s, v2u p) {
    v2u blockpos = CELL_TO_BLOCK(p);
    u32 blockid = BlockInsert(s, blockpos, UINT32_MAX);
    Block* block = &s->blockpool[blockid];

    //ensure block exists
    if (!block) {
        return;
    }

    v2u offset = CELL_TO_OFFSET(p);
    u32 index = offset.x + offset.y * BLOCK_SIZE;

    if (block->cells[index].t != CT_EMPTY) {
        block->nonempty--;
    }

    if (block->nonempty > 0) return;
    //if the block is completely empty

    BlockDelete(s, blockpos);
}

void SpreadSheetFree(SpreadSheet* s) {
    Free(s->mem, s->blockpool, s->bcap * sizeof(Block));
    Free(s->mem, s->freestatus, s->bcap * sizeof(u32));
    Free(s->mem, s->keys, s->cap * sizeof(v2u));
    Free(s->mem, s->values, s->cap * sizeof(u32));
}
