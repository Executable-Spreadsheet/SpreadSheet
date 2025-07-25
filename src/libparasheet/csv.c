#include "libparasheet/csv.h"

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

// === Parse One Line of CSV ===

int csv_parse_line(StringTable* str, const char* line, u32 linesize, CellValue* out_values, size_t max_values) {
    char buf[1024];
    strncpy(buf, line, sizeof(buf));
    buf[sizeof(buf) - 1] = '\0';

    char* token = strtok(buf, ",");
    size_t count = 0;

    while (token && count < max_values) {
        token = trim(token);
        CellValue v;

        if (is_integer(token)) {
            v.t = CT_INT;
            v.d.i = atoi(token);
        } else if (is_float(token)) {
            v.t = CT_FLOAT;
            v.d.f = (float)atof(token);
        } else {
            v.t = CT_TEXT;
            i8* copy = Alloc(str->mem, strlen(token));
            strcpy((char*)copy, token);
            v.d.index = StringAdd(str, copy);  // string table index
        }

        out_values[count++] = v;
        token = strtok(NULL, ",");
    }

    return (int)count;
}

// === Load Entire CSV File ===

bool csv_load_file(FILE* csv, StringTable* str, SpreadSheet* sheet) {
    Allocator a = GlobalAllocatorCreate();

    struct stat info;
    if (fstat(fileno(csv), &info)) {
        err("Failed to stat file");
        panic();
    }

    SString file = {.size = info.st_size, .data = Alloc(a, info.st_size)};

    fread(file.data, 1, info.st_size, csv);
    fclose(csv);

    u32 row = 0;
    char* cursor = (char*)file.data;
    char* line_start = cursor;

    while (cursor - (char*)file.data < file.size) {
        if (*cursor == '\n' || *cursor == '\r') {
            *cursor = '\0';

            CellValue values[256];
            int count = csv_parse_line(str, line_start, strlen(line_start), values, 256);

            for (int col = 0; col < count; col++) {
                v2u p = { .x = (u32)col, .y = row };
                SpreadSheetSetCell(sheet, p, values[col]);
            }

            row++;
            cursor++;
            if (*cursor == '\n' || *cursor == '\r') cursor++;
            line_start = cursor;
        } else {
            cursor++;
        }
    }

    // Final line (in case no newline at EOF)
    if (cursor != line_start) {
        CellValue values[256];
        int count = csv_parse_line(str, line_start, strlen(line_start), values, 256);
        for (int col = 0; col < count; col++) {
            v2u p = { .x = (u32)col, .y = row };
            SpreadSheetSetCell(sheet, p, values[col]);
        }
    }

    Free(a, file.data, file.size);

    return true;
}

// === Export CSV File ===

void csv_export_file(Allocator a, const char* filename, SpreadSheet* sheet, StringTable* str) {
	SString out = {.data = Alloc(a, MB(1)), .size = 0};
	i8* cursor = out.data;

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
						cursor += sprintf((char *)cursor, "%d", val->d.i);
						break;
					case CT_FLOAT:
						cursor += sprintf((char *)cursor, "%f", val->d.f);
						break;
                    case CT_TEXT: {
                        SString text = StringGet(str, val->d.index);
                        cursor += sprintf((char *)cursor, "%.*s", text.size, text.data);
                    } break;
					default:
						break;
				}
			}

			if (x + 1 < maxx)
				*cursor++ = ',';
		}
		*cursor++ = '\n';
	}

	out.size = (u32)(cursor - out.data);
	WriteFileS(filename, out);
	Free(a, out.data, MB(1));
}
