#include <libparasheet/lib_internal.h>
#include <stdint.h>
#include <string.h>
#include <util/util.h>

//NOTE(ELI): This declaration is here for recursive purposes. The Resize function
//depends on the insertion function and the insertion function always calls
//the resize function first. This forward declaration is therefore required.
//
//I could have separated them so that there is a version of insertion which calls
//resize and a version which doesn't but it didn't seem necessary at the moment,
//but feel free in the future!!
static u32 StringAddInternal(StringTable* table, SString string, u32 sidx);

/*
* Allocates a unique index to a string. Will automatically resize
* the arrays keeping track of each string. Internal use only
*/
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
        

        table->gen = Realloc(table->mem,
                             table->gen, oldsize * sizeof(u32),
                             table->scap * sizeof(u32));

        for (i32 i = table->scap - 1; i >= (i32)oldsize; i--) {
            table->freelist[table->fsize++] = i;
            table->gen[i] = 0;
        }
    }

    u32 idx = table->freelist[--table->fsize];
    table->strings[idx] = s;
    return idx; 
}


/*
* Resizes the hash table, doubles the size and
* copies each element over. Internal use only
*/
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
            StringAddInternal(table, table->strings[oldvals[i]], oldvals[i]);
        }
    }


    Free(table->mem, oldvals, oldsize * sizeof(u32));
    Free(table->mem, oldmeta, oldsize * sizeof(u32));
}

/*
* Internal implementation of string insertion. The additional sidx
* parameter is used for reinserting strings when resizing and is hidden
* from the standard api functions.
*/
static u32 StringAddInternal(StringTable* table, SString string, u32 sidx) {
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


StrID StringAdd(StringTable* table, i8* string) {
    SString sized = {
        .data = string,
        .size = strlen((char*)string)
    };
    StrID str = {0};
    str.idx = StringAddInternal(table, sized, UINT32_MAX);
    str.gen = table->gen[str.idx];
    return str;
}

StrID StringAddS(StringTable* table, SString string) {
    StrID str = {0};
    str.idx = StringAddInternal(table, string, UINT32_MAX);
    str.gen = table->gen[str.idx];
    return str;
}

SString StringDel(StringTable* table, StrID sid) {
    u32 index = sid.idx;
    u32 idx = table->entry[index];

    table->freelist[table->fsize++] = index;
    
    table->meta[idx] = UINT32_MAX;
    table->size--;

    SString output = table->strings[index];
    table->gen[index]++;

    for (u32 i = 0; i < table->cap; i++) {
        u32 next = (idx + 1) % table->cap;
        
        if (table->meta[next] == 0 || table->meta[next] == UINT32_MAX) {
            return output;  
        }

        table->meta[idx] = table->meta[next] - 1;
        table->vals[idx] = table->vals[next];

        table->meta[next] = UINT32_MAX;

        idx = next;
    }

    panic(); 
}

const SString StringGet(StringTable* table, StrID sid) {
    if (table->gen[sid.idx] == sid.gen) return table->strings[sid.idx];
    else return (SString){.size = 0, .data = NULL};
}

void StringFree(StringTable* table) {
    Free(table->mem, table->vals, table->cap * sizeof(u32));
    Free(table->mem, table->meta, table->cap * sizeof(u32));

    Free(table->mem, table->strings, table->scap * sizeof(SString));
    Free(table->mem, table->freelist, table->scap * sizeof(u32));
    Free(table->mem, table->entry, table->scap * sizeof(u32));
    Free(table->mem, table->gen, table->scap * sizeof(u32));
}
