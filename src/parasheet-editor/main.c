#include "libparasheet/lib_internal.h"
#include <stdint.h>
#include <stdio.h>
#include <util/util.h>
#include <ncurses.h>


/*
+---------------------------------------------------------------+
|    INFO(ELI): Just for simplicity, We should start but        |
|    putting all the functions in the same file, we can         |
|    break them out later if it gets too long and unweildy      |
|    but for right now keeping things as simple as possible     |
|    will make things faster.                                   |
+---------------------------------------------------------------+
*/

#define CELL_WIDTH 10
#define CELL_HEIGHT 2



void drawBox(v2u pos, v2u size, SString s);
SString CellDisplay(Allocator a, SpreadSheet* s, v2u pos, u32 maxlen);


int main(int argc, char* argv[]) {
    initscr();
    noecho();               //prevent echoing output
    raw();                  //remove buffering
    keypad(stdscr, TRUE);   //enable extended keys
    set_escdelay(0);        //remove delay when processing esc
    curs_set(0);            //set cursor to be invisible


    SpreadSheet sheet = {
        .mem = GlobalAllocatorCreate(),
    };

    Allocator stack = StackAllocatorCreate(sheet.mem, KB(1));

    //set logging
    logfile = fopen("log.out", "w+");

    //persistant
    u8 ch = 0;
    v2i base = {0, 0};
    v2i cursor = {0, 0};

    while (1) {
        clear();
        //rendering

        for (u32 i = 0; i < COLS/CELL_WIDTH + 1; i++) {
            for (u32 j = 0; j < LINES/CELL_HEIGHT + 1; j++) {
                if (i == cursor.x && j == cursor.y) {
                    continue;
                }
                SString info = CellDisplay(stack, &sheet, (v2u){base.x + i, base.y + j}, CELL_WIDTH - 2);
                drawBox((v2u){i * CELL_WIDTH, j * CELL_HEIGHT}, (v2u){CELL_WIDTH, CELL_HEIGHT}, info);
                StackAllocatorReset(&stack);
            }
        }

        attron(A_REVERSE);
        SString info = CellDisplay(stack, &sheet, (v2u){cursor.x, cursor.y}, CELL_WIDTH - 2);
        drawBox((v2u){cursor.x * CELL_WIDTH, cursor.y * CELL_HEIGHT}, (v2u){CELL_WIDTH, CELL_HEIGHT}, info);
        StackAllocatorReset(&stack);

        mvprintw(LINES - 2, 0, "Cursor (%d %d)  ch: %d", cursor.x, cursor.y, ch);

        refresh();

        //updating
        ch = getch();
        if (ch == 27) break;

        switch(ch) {
            case 'h': { cursor.x -= 1; } break;
            case 'j': { cursor.y += 1; } break;
            case 'k': { cursor.y -= 1; } break;
            case 'l': { cursor.x += 1; } break;
            default: break;
        }

        cursor.x = MAX(cursor.x, 0);
        cursor.y = MAX(cursor.y, 0);
    }


    endwin();
    return 0;
}

SString CellDisplay(Allocator a, SpreadSheet* s, v2u pos, u32 maxlen) {
    CellValue* cell = SpreadSheetGetCell(s, pos);

    if (!cell) {
        return (SString){NULL, 0};
    }

    SString value = {NULL, 0};
    value.data = Alloc(a, maxlen);
    char* fmt = "";

    switch (cell->t) {
        case CT_EMPTY: break;
        case CT_FLOAT: { fmt = "%f"; } break;
        case CT_INT: { fmt = "%d"; } break;

        default: { endwin(); todo(); } break;
    }

    value.size = snprintf((char*)value.data, maxlen, fmt, cell->d);
    return value;
}

void drawBox(v2u pos, v2u size, SString s) {
    
    mvaddch(pos.y + 0,      pos.x + 0, '+');
    mvaddch(pos.y + 0,      pos.x + size.x, '+');
    mvaddch(pos.y + size.y, pos.x + 0, '+');
    mvaddch(pos.y + size.y, pos.x + size.x, '+');

    mvhline(pos.y + 0,      pos.x + 1, '-', size.x - 1);
    mvhline(pos.y + size.y, pos.x + 1, '-', size.x - 1);

    mvvline(pos.y + 1, pos.x + 0,      '|', size.y - 1);
    mvvline(pos.y + 1, pos.x + size.x, '|', size.y - 1);

    attrset(A_NORMAL);
    mvprintw(pos.y + size.y/2, pos.x + 1, "%.*s", s.size, s.data);
}
