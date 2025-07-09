#include "libparasheet/csv.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

#include "libparasheet/lib_internal.h"         // for CellValue, v2u, SpreadSheet, SpreadSheetSetCell
#include "util/util.h"               // for DumpFile, Allocator

// === Parse Helpers ===

static void trim(char* str) {
  char* end;
  while (isspace((unsigned char)*str)) str++;
  if (*str == 0) return;
  end = str + strlen(str) - 1;
  while (end > str && isspace((unsigned char)*end)) end--;
  *(end + 1) = 0;
}

static bool is_integer(const char* s) {
  if (*s == '-' || *s == '+') s++;
  while (*s) {
    if (!isdigit(*s)) return false;
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
      v.d.index = 0;  // string table index
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
    int count = csv_parse_line(line_start, values, 256);
    for (int col = 0; col < count; col++) {
      v2u p = { .x = (u32)col, .y = row };
      SpreadSheetSetCell(sheet, p, values[col]);
    }
  }

  return true;
}
