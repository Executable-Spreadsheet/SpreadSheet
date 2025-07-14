#include <libparasheet/lib_internal.h>
#include <stdint.h>
#include <string.h>
#include <util/util.h>

static u32 StringAddv(StringTable* table, SString string, u32 sidx);

static u32 AllocString(StringTable* table, SString s) {
    if (table->fsize == 0) {
        u32 oldsize = table->scap;
        table->scap = table->scap ? table->scap * 2 : 4;
        table->strings = Realloc(table->mem,
                                 table->strings, oldsize * sizeof(SString),
                                 table->scap * sizeof(SString));

        table->freelist = Realloc(table->mem,
                                  table->freelist, oldsize * sizeof(u32),
                                  table->scap * sizeof(u32));

        table->entry = Realloc(table->mem,
                               table->entry, oldsize * sizeof(u32),
                               table->scap * sizeof(u32));
        
        for (i32 i = table->scap - 1; i >= (i32)oldsize; i--) {
            table->freelist[table->fsize++] = i;
        }
    }

    u32 idx = table->freelist[--table->fsize];
    table->strings[idx] = s;
    return idx; 
}


static void StringResize(StringTable* table) {
    if (table->size + 1 <= (table->cap * MAX_LOAD_FACTOR))
        return;


    u32* oldvals = table->vals;
    u32* oldmeta = table->meta;
    u32 oldsize = table->cap;

    table->cap = table->cap ? table->cap * 2 : 4;
    table->meta = Alloc(table->mem, table->cap * sizeof(u32));
    table->vals = Alloc(table->mem, table->cap * sizeof(u32));

    memset(table->meta, -1, table->cap * sizeof(u32));
    memset(table->vals, 0,  table->cap * sizeof(u32));

    table->size = 0;

    for (u32 i = 0; i < oldsize; i++) {
        if (oldmeta[i] != UINT32_MAX) {
            StringAddv(table, table->strings[oldvals[i]], oldvals[i]);
        }
    }


    Free(table->mem, oldvals, oldsize * sizeof(u32));
    Free(table->mem, oldmeta, oldsize * sizeof(u32));
}

static u32 StringAddv(StringTable* table, SString string, u32 sidx) {
    StringResize(table);

    u32 idx = hash((u8*)string.data, string.size) % table->cap;
    u32 curr_dist = 0;

    for (u32 i = 0; i < table->cap; i++) {
        u32 dist = table->meta[idx];

        if (dist == UINT32_MAX) {
            table->meta[idx] = curr_dist;

            if (sidx == UINT32_MAX) 
                sidx = AllocString(table, string);

            table->vals[idx] = sidx;
            table->entry[sidx] = idx;

            table->size++;
            return table->vals[idx];
        }

        if (dist < curr_dist) {
            u32 temp = table->vals[idx];
            u32 mtemp = table->meta[idx];

            if (sidx == UINT32_MAX) 
                sidx = AllocString(table, string);
            
            table->vals[idx] = sidx;
            table->entry[sidx] = idx;
            table->meta[idx] = curr_dist;

            curr_dist = mtemp;
            sidx = temp;
        }

        if (SStrCmp(string, table->strings[table->vals[idx]]) == 0) {
            return table->vals[idx];
        }


        idx = (idx + 1) % table->cap;
        curr_dist++;
    }
    panic();
}


u32 StringAdd(StringTable* table, i8* string) {
    SString sized = {
        .data = string,
        .size = strlen((char*)string)
    };
    return StringAddv(table, sized, UINT32_MAX);
}

u32 StringAddS(StringTable* table, SString string) {
    return StringAddv(table, string, UINT32_MAX);
}

SString StringDel(StringTable* table, u32 index) {
    u32 idx = table->entry[index];

    table->freelist[table->fsize++] = index;
    
    table->meta[idx] = UINT32_MAX;
    table->size--;

    SString output = table->strings[index];

    for (u32 i = 0; i < table->cap; i++) {
        u32 next = (idx + 1) % table->cap;
        
        if (table->meta[next] == 0 ||
            table->meta[next] == UINT32_MAX) return output;  

        table->meta[idx] = table->meta[next] - 1;
        table->vals[idx] = table->vals[next];

        table->meta[next] = UINT32_MAX;

        idx = next;
    }

    panic(); 
}

void StringFree(StringTable* table) {
    Free(table->mem, table->vals, table->cap * sizeof(u32));
    Free(table->mem, table->meta, table->cap * sizeof(u32));

    Free(table->mem, table->strings, table->scap * sizeof(SString));
    Free(table->mem, table->freelist, table->scap * sizeof(u32));
    Free(table->mem, table->entry, table->scap * sizeof(u32));
}
