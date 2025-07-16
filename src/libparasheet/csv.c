#include "libparasheet/csv.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libparasheet/lib_internal.h" // for CellValue, SpreadSheet, SpreadSheetSetCell
#include "util/util.h"				   // for DumpFile, Allocator, v2u

// === Parse Helpers ===

static void trim(char* str) {
	char* end;
	while (isspace((unsigned char)*str))
		str++;
	if (*str == 0)
		return;
	end = str + strlen(str) - 1;
	while (end > str && isspace((unsigned char)*end))
		end--;
	*(end + 1) = 0;
}

static bool is_integer(const char* s) {
	if (*s == '-' || *s == '+')
		s++;
	while (*s) {
		if (!isdigit(*s))
			return false;
		s++;
	}
	return true;
}

static bool is_float(const char* s) {
	char* endptr;
	strtod(s, &endptr);
	return endptr != s && *endptr == '\0';
}

// === Parse One Line of CSV ===

int csv_parse_line(const char* line, CellValue* out_values, size_t max_values) {
	char buf[1024];
	strncpy(buf, line, sizeof(buf));
	buf[sizeof(buf) - 1] = '\0';

	char* token = strtok(buf, ",");
	size_t count = 0;

	while (token && count < max_values) {
		trim(token);
		CellValue v;

		if (is_integer(token)) {
			v.t = CT_INT;
			v.d.i = atoi(token);
		} else if (is_float(token)) {
			v.t = CT_FLOAT;
			v.d.f = (float)atof(token);
		} else {
			v.t = CT_TEXT;
			v.d.index = 0; // string table index
		}

		out_values[count++] = v;
		token = strtok(NULL, ",");
	}

	return (int)count;
}

// === Load Entire CSV File ===

bool csv_load_file(const char* filename, SpreadSheet* sheet) {
	Allocator a = GlobalAllocatorCreate();
	SString file = DumpFile(a, filename);

	u32 row = 0;
	char* cursor = (char*)file.data;
	char* line_start = cursor;

	while (*cursor) {
		if (*cursor == '\n' || *cursor == '\r') {
			*cursor = '\0';

			CellValue values[256];
			int count = csv_parse_line(line_start, values, 256);

			for (int col = 0; col < count; col++) {
				v2u p = {.x = (u32)col, .y = row};
				SpreadSheetSetCell(sheet, p, values[col]);
			}

			row++;
			cursor++;
			if (*cursor == '\n' || *cursor == '\r')
				cursor++;
			line_start = cursor;
		} else {
			cursor++;
		}
	}

	// Final line (in case no newline at EOF)
	if (cursor != line_start) {
		CellValue values[256];
		int count = csv_parse_line(line_start, values, 256);
		for (int col = 0; col < count; col++) {
			v2u p = {.x = (u32)col, .y = row};
			SpreadSheetSetCell(sheet, p, values[col]);
		}
	}

	return true;
}

// === Export CSV File ===

void csv_export_file(const char* filename, SpreadSheet* sheet) {
	Allocator a = GlobalAllocatorCreate();
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
					case CT_TEXT:
						cursor += sprintf((char *)cursor, "%u", val->d.index);
						break;
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
	WriteFileS(out);
	Free(a, out.data, MB(1));
}
