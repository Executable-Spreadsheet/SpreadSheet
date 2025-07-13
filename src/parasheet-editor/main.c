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
SString CellDisplay(Allocator a, SpreadSheet* s, v2u pos);


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

    clear();
    u8 c = 0;
    while (1) {
        clear();
        //rendering

        v2u base = {0, 0};
        for (u32 i = 0; i < COLS/CELL_WIDTH + 1; i++) {
            for (u32 j = 0; j < LINES/CELL_HEIGHT + 1; j++) {
                SString info = CellDisplay(stack, &sheet, (v2u){base.x + i, base.y + j});
                drawBox((v2u){i * CELL_WIDTH, j * CELL_HEIGHT}, (v2u){CELL_WIDTH, CELL_HEIGHT}, info);
            }
        }

        refresh();

        //updating
        c = getch();
        if (c == 27) break;
    }


    endwin();

    return 0;
}

SString CellDisplay(Allocator a, SpreadSheet* s, v2u pos) {
    CellValue* c = SpreadSheetGetCell(s, pos);

    if (!c) {
        return (SString){NULL, 0};
    }

    todo();

}

void drawBox(v2u pos, v2u size, SString s) {
    
    mvaddch(pos.y + 0,      pos.x + 0, '+');
    mvaddch(pos.y + 0,      pos.x + size.x, '+');
    mvaddch(pos.y + size.y, pos.x + 0, '+');
    mvaddch(pos.y + size.y, pos.x + size.x, '+');

    mvprintw(pos.y + size.y/2, pos.x + 1, "%.*s", s.size, s.data);

    mvhline(pos.y + 0,      pos.x + 1, '-', size.x - 1);
    mvhline(pos.y + size.y, pos.x + 1, '-', size.x - 1);

    mvvline(pos.y + 1, pos.x + 0,      '|', size.y - 1);
    mvvline(pos.y + 1, pos.x + size.x, '|', size.y - 1);

}
