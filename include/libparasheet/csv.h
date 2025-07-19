#ifndef CSV_H
#define CSV_H

#include "libparasheet/lib_internal.h" // for SpreadSheet, CellValue
#include <stdbool.h>
#include <stddef.h> // for size_t

/*
+------------------------------------------------------------+
|   CSV Parser Interface                                     |
|   Author: RiceBoy 🐸                                        |
|                                                            |
|   Provides a simple interface for parsing CSV strings      |
|   and loading CSV files into a spreadsheet data structure. |
+------------------------------------------------------------+
*/

/**
 * Parses a single CSV line into CellValue array.
 *
 * @param line        A null-terminated CSV line.
 * @param out_values  Output array of CellValues.
 * @param max_values  Maximum number of values to write into out_values.
 * @return            Number of values parsed.
 */
int csv_parse_line(StringTable* str, const char* line, u32 linesize, CellValue* out_values, size_t max_values);

/**
 * Loads an entire CSV file into the given spreadsheet.
 *
 * @param filename  Path to the CSV file.
 * @param sheet     Pointer to the target SpreadSheet to populate.
 * @return          True on success, false on failure.
 */
bool csv_load_file(FILE* csv, StringTable* str, SpreadSheet* sheet);

#endif // CSV_H
