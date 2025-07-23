#include "libparasheet/csv.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "libparasheet/lib_internal.h" // for CellValue, SpreadSheet, SpreadSheetSetCell
#include "util/util.h"				   // for DumpFile, Allocator, v2u

// === Parse Helpers ===
static char* trim(char* str) {
    char* end;
    while (isspace(str[0])) str++;
    if (*str == 0) return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace(end[0])) end--;
    *(end + 1) = 0;
    return str;
}

bool is_integer(const char* s) {
    if (*s == '-' || *s == '+') s++;
    while (*s) {
        if (!isdigit(*s)) return false;
        s++;
    }
    return true;
}

bool is_float(const char* s) {
    char* endptr;
    strtod(s, &endptr);
    return endptr != s && *endptr == '\0';
}

// === Parse One Cell of CSV ===
static u32 parse_value(char* cursor, SpreadSheet* sheet, v2u pos) {
    u32 i = 0;
    u32 integer = 0;

    CellValue data = {0};
	while (isdigit(cursor[i])) {
		integer = integer * 10;
		integer += (cursor[i] - '0');
		i += 1;
	}
	data.d.i = integer;
	if (cursor[i] != '.') {
        data.t = CT_INT;
        SpreadSheetSetCell(sheet, pos, data); 
        return i;
    }

    f32 frac = (f32)integer;
    f32 multiplier = 0.1;
    i += 1;
    while (isdigit(cursor[i])) {
        frac += (f32)(cursor[i] - '0') * multiplier;
        multiplier = multiplier * 0.1;
        i += 1;
    }
    data.d.f = frac;

    data.t = CT_FLOAT;
    SpreadSheetSetCell(sheet, pos, data); 


    return i + 1;
}

static u32 parse_text(char* cursor, SpreadSheet* sheet, StringTable* str, v2u pos) {
    cursor++;
    u32 i = 0;
    while (cursor[i] != '\"') {
        if (cursor[i] == '\\' && cursor[i + 1] == '\"') {
            i++;
        }
        i++;
    }

    i8* cpy = Alloc(sheet->mem, (i + 1) * sizeof(i8));
    memset(cpy, 0, (i+1) * sizeof(i8));
    u32 c = 0;
    for (u32 j = 0; j < i; j++) {
        if (cursor[j] == '\\' && cursor[j + 1] == '\"') {
            j++;
        }
        cpy[c++] = cursor[j];
    }

    CellValue val = {
        .t = CT_TEXT,
        .d.index = StringAdd(str, cpy)
    };
    SpreadSheetSetCell(sheet, pos, val);

    return i + 2;
}

// === Load Entire CSV File ===

bool csv_load_file(FILE* csv, StringTable* str, SpreadSheet* sheet) {
    Allocator a = GlobalAllocatorCreate();

    struct stat info;
    if (fstat(fileno(csv), &info)) {
        err("Failed to stat file: %n", strerror(errno));
        return false;
    }

    SString file = {.size = info.st_size, .data = Alloc(a, info.st_size)};

    fread(file.data, 1, info.st_size, csv);
    fclose(csv);

    char* cursor = (char*)file.data;
    v2u pos = {0, 0};

    while (cursor - (char*)file.data < file.size) {
        while (isspace(cursor[0]) && cursor[0] != '\n') cursor++;
        char c = cursor[0];

        switch (c) {
            case ',':
                {
                    pos.x++;
                    cursor++;
                    continue;
                } break;
            case '\n':
                {
                    pos.y++;
                    pos.x = 0;
                    cursor++;
                    continue;
                } break;
            default:
                {
                    if (cursor[0] == '\"') cursor += parse_text(cursor, sheet, str, pos);
                    else if (isdigit(cursor[0])) cursor += parse_value(cursor, sheet, pos);
                } break;
        }
    }

    Free(a, file.data, file.size);

    return true;
}


// === Export CSV File ===

void csv_write_string(FILE* out, SString s) {
    
    fputc('\"', out);

    for (u32 i = 0; i < s.size; i++) {
        if (s.data[i] == '\"') {
            fputc('\\', out);
            fputc(s.data[i], out);
        } else {
            fputc(s.data[i], out);
        }
    }

    fputc('\"', out);

}


void csv_export_file(const char* filename, SpreadSheet* sheet, StringTable* str) {

    FILE* out = fopen(filename, "w+");

	u32 maxx = 0;
	u32 maxy = 0;
    v2u Invalid = {UINT32_MAX, UINT32_MAX};

	for (u32 i = 0; i < sheet->cap; i++) {
		if (!CMPV2(sheet->keys[i], Invalid)) {
			v2u pos = sheet->keys[i];
			if (pos.x * BLOCK_SIZE > maxx) maxx = pos.x * BLOCK_SIZE;
			if (pos.y * BLOCK_SIZE > maxy) maxy = pos.y * BLOCK_SIZE;
		}
	}
	maxx += BLOCK_SIZE;
	maxy += BLOCK_SIZE;

	for (u32 y = 0; y < maxy; y++) {
		for (u32 x = 0; x < maxx; x++) {
			v2u pos = {.x = x, .y = y};
			CellValue* val = SpreadSheetGetCell(sheet, pos);

			if (val && val->t != CT_EMPTY) {
				switch (val->t) {
					case CT_INT:
						fprintf(out, "%d", val->d.i);
						break;
					case CT_FLOAT:
						fprintf(out, "%f", val->d.f);
						break;
                    case CT_TEXT: {
                        SString text = StringGet(str, val->d.index);
                        csv_write_string(out, text);
                    } break;
					default:
						break;
				}
			}

			if (x + 1 < maxx)
				fputc(',', out);
		}
        fputc('\n', out);
	}

    fclose(out);
}
