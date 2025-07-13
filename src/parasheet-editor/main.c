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

// VISUAL TUNING
#define CELL_WIDTH 10
#define CELL_HEIGHT 2

// ASCII VALUES
#define KEY_ESCAPE 27
#define KEY_ENTER_REAL 10

void drawBox(v2u pos, v2u size, SString str);
SString cellDisplay(Allocator mem, SpreadSheet* sheet, v2u pos, u32 maxlen);

// supplies the currently selected keybinds
struct KeyBinds {
    u8 cursor_up;
    u8 cursor_down;
    u8 cursor_left;
    u8 cursor_right;
    u8 terminal;
    u8 exit; // quit the program
    u8 edit; // open a cell for editing
    u8 nav;  // go back to normal mode
};
typedef struct KeyBinds KeyBinds;

// keybinds are currently hard set but that can change later
KeyBinds keybinds_hjkl = {
    .cursor_up = 'k',
    .cursor_down = 'j',
    .cursor_left = 'h',
    .cursor_right = 'l',
    .terminal = ':',
    .exit = 'q',
    .edit = (u8)KEY_ENTER_REAL,
    .nav = (u8)KEY_ESCAPE
};
KeyBinds keybinds_wasd = {
    .cursor_up = 'w',
    .cursor_down = 's',
    .cursor_left = 'a',
    .cursor_right = 'd',
    .terminal = ':',
    .exit = 'q',
    .edit = (u8)KEY_ENTER_REAL,
    .nav = (u8)KEY_ESCAPE
};

enum EditorState {
    NORMAL,
    TERMINAL,
    EDIT,
    SHUTDOWN
};
typedef enum EditorState EditorState;

struct RenderHandler {
    u8 ch;
    v2i base;
    v2i cursor;
    KeyBinds keybinds;
    EditorState state;
};
typedef struct RenderHandler RenderHandler;

// clarise: can i make this a SString later?
char* stateToString(EditorState state){
    if (state == NORMAL){return "NORMAL";}
    else if (state == TERMINAL){return "TERMINAL";}
    else if (state == EDIT){return "EDIT";}
    else if (state == SHUTDOWN){return "SHUTDOWN";}
    else {return "INVALID";}
}

void boundCursor(RenderHandler* handler){
        handler->cursor.x = MAX(handler->cursor.x, 0);
        handler->cursor.y = MAX(handler->cursor.y, 0);
}

void handleKey(RenderHandler* handler){
    char keyIn = handler->ch;
    switch (handler->state){
        case NORMAL:
            if (keyIn == handler->keybinds.cursor_up) {
            handler->cursor.y -= 1;
             }
            else if (keyIn == handler->keybinds.cursor_down) {
                handler->cursor.y += 1;
            }
            else if (keyIn == handler->keybinds.cursor_left) {
                handler->cursor.x -= 1;
            }
            else if (keyIn == handler->keybinds.cursor_right) {
                handler->cursor.x += 1;
            } else if (keyIn == handler->keybinds.exit) {
                handler->state = SHUTDOWN;
            }
            else if (keyIn == handler->keybinds.terminal) {
                handler->state = TERMINAL;
            }
            else if (keyIn == handler->keybinds.edit) {
                handler->state = EDIT;
            }
            break;
        case EDIT:
            // implement input to cell
            if (keyIn == handler->keybinds.nav) {
                handler->state = NORMAL;
            }
            break;
        case TERMINAL:
            if (keyIn == handler->keybinds.nav) {
                handler->state = NORMAL;
            }
            break;
        case SHUTDOWN:
            // no behavior; silencing compiler warning
            break;
    }

    // cleanup
    boundCursor(handler);
}


int main(int argc, char* argv[]) {
    // ncurses setup
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

    // if you want different keybinds u change that here
    RenderHandler handler = {
        .ch = 0,
        .base = {0, 0},
        .cursor = {0, 0},
        .state = NORMAL,
        .keybinds = keybinds_wasd
    };

    // rendering loop
    while (1) {
        clear();
        //rendering

        for (u32 i = 0; i < COLS/CELL_WIDTH + 1; i++) {
            for (u32 j = 0; j < LINES/CELL_HEIGHT + 1; j++) {
                if (i == handler.cursor.x && j == handler.cursor.y) {
                    continue;
                }
                SString info = cellDisplay(stack, &sheet, 
                        (v2u){handler.base.x + i, handler.base.y + j}, CELL_WIDTH - 2);
                drawBox((v2u){i * CELL_WIDTH, j * CELL_HEIGHT}, 
                        (v2u){CELL_WIDTH, CELL_HEIGHT}, info);
                StackAllocatorReset(&stack);
            }
        }

        // currently selected cell
        attron(A_REVERSE);
        SString info = cellDisplay(stack, &sheet, (v2u)
                {handler.cursor.x, handler.cursor.y}, CELL_WIDTH - 2);
        drawBox((v2u){handler.cursor.x * CELL_WIDTH, handler.cursor.y * CELL_HEIGHT},
                (v2u){CELL_WIDTH, CELL_HEIGHT}, info);
        StackAllocatorReset(&stack);

        mvprintw(LINES - 2, 0, "Cursor (%d %d)  ch: %d State: %s", 
                handler.cursor.x, handler.cursor.y, handler.ch, stateToString(handler.state));

        refresh();

        //updating
        handler.ch = getch();
        
        handleKey(&handler);
        if (handler.state == SHUTDOWN) break;

    }


    endwin();
    return 0;
}

SString cellDisplay(Allocator mem, SpreadSheet* sheet, v2u pos, u32 maxlen) {
    CellValue* cell = SpreadSheetGetCell(sheet, pos);

    if (!cell) {
        return (SString){NULL, 0};
    }

    SString value = {NULL, 0};
    value.data = Alloc(mem, maxlen);
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

void drawBox(v2u pos, v2u size, SString str) {
    
    mvaddch(pos.y + 0,      pos.x + 0, '+');
    mvaddch(pos.y + 0,      pos.x + size.x, '+');
    mvaddch(pos.y + size.y, pos.x + 0, '+');
    mvaddch(pos.y + size.y, pos.x + size.x, '+');

    mvhline(pos.y + 0,      pos.x + 1, '-', size.x - 1);
    mvhline(pos.y + size.y, pos.x + 1, '-', size.x - 1);

    mvvline(pos.y + 1, pos.x + 0,      '|', size.y - 1);
    mvvline(pos.y + 1, pos.x + size.x, '|', size.y - 1);

    attrset(A_NORMAL);
    mvprintw(pos.y + size.y/2, pos.x + 1, "%.*s", str.size, str.data);
}
