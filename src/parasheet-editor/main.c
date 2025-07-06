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

int main(int argc, char* argv[]) {
    initscr();
    noecho();               //prevent echoing output
    raw();                  //remove buffering
    keypad(stdscr, TRUE);   //enable extended keys
    set_escdelay(0);        //remove delay when processing esc

    clear();
    u8 c = 0;
    while (1) {
        clear();
        //rendering

        printw("Hello World!\n");
        move(1, 0);
        printw("Current Screen size: (%d, %d)", LINES, COLS);

        refresh();

        //updating
        c = getch();
        if (c == 27) break;
    }


    endwin();
    return 0;
}

