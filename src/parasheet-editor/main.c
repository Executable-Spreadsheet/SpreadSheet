#include "libparasheet/lib_internal.h"
#include "libparasheet/csv.h"
#include <linux/limits.h>
#include <util/util.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <ncurses.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/inotify.h>


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

#define MARGIN_TOP 1
#define MARGIN_LEFT 2

// ASCII VALUES
#define KEY_ESCAPE 27
#define KEY_ENTER_REAL 10

//NOTE(ELI): I would recommend using PATH_MAX for this
//rather than a custom macro. PATH_MAX is the maximum length of a path in
//the filesystem so it garuntees any executable is at most that size.
#define STRING_SIZE PATH_MAX + 1
#define CONFIG_FILEPATH "config.txt"


void drawBox(v2u pos, v2u size, SString str);
SString cellDisplay(SpreadSheet* sheet, StringTable* str, v2u pos, u32 maxlen);

// supplies the currently selected keybinds
typedef struct KeyBinds {
    u8 cursor_up;
    u8 cursor_down;
    u8 cursor_left;
    u8 cursor_right;
    u8 terminal;
    u8 exit; // quit the program
    u8 edit; // open a cell for editing
    u8 nav;  // go back to normal mode
} KeyBinds;


// store a file reference with position of data
typedef struct EditHandles {
    u32 notify; //inotify context handle
    u32* fd;    //specific file watches
    v2u* idx;   //associate files with cells
    u32 size;
    u32 cap;
} EditHandles;

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

typedef enum EditorState {
    NORMAL,
    TERMINAL,
    EDIT,
    SHUTDOWN
} EditorState;

typedef struct RenderHandler {
    u8 ch;
    v2i base;
    v2i cursor;
    EditHandles edit;
    EditorState state;
    SpreadSheet* sheet;
    SpreadSheet* files;
    StringTable* str;
    KeyBinds keybinds;
    u8 preferred_terminal[STRING_SIZE];
    u8 preferred_text_editor[STRING_SIZE];
} RenderHandler;

// NOTE(ELI): I made this an SString and converted it to a switch
// since that seemed better for formating and readability.
SString stateToString(EditorState state){
    switch (state) {
        case NORMAL:    return sstring("NORMAL");
        case TERMINAL:  return sstring("TERMINAL");
        case EDIT:      return sstring("EDIT");
        case SHUTDOWN:  return sstring("SHUTDOWN");
        default:        return sstring("INVALID");
    }
}

void editCell(RenderHandler * handler){
    // somehow deduplicate open cells
    // create new tempfile for cell
    // open vim on tempfile
    // TODO

    log("wasd: %n, %n", handler->preferred_text_editor, handler->preferred_terminal);
    //You can run this as system, however I suspect it would be better to run as a fork-execl style instead.

    v2u pos = {handler->base.x + handler->cursor.x, handler->base.y + handler->cursor.y};

    CellValue* data = SpreadSheetGetCell(handler->sheet, pos);
    CellValue* file = SpreadSheetGetCell(handler->files, pos);

    char filename[PATH_MAX + 1] = {0};
    snprintf(filename, PATH_MAX, "cell%d%d", pos.x, pos.y);
    //snprintf(filename, PATH_MAX, "test");

    if (!file || file->t == CT_EMPTY) {
        CellValue val = {0};
        val.d.i = open(filename, O_CREAT | O_RDWR | O_TRUNC, 0666);
        val.t = CT_INT;
        SpreadSheetSetCell(handler->files, pos, val);

        //inotify_add_watch(handler->notify, filename, IN_MODIFY);

        file = SpreadSheetGetCell(handler->files, pos);
    }

    if (data) {
        FILE* stream = fdopen(file->d.i, "w");
        switch (data->t) {
            case CT_INT: { print(stream, "%d", data->d.i); } break;
            case CT_FLOAT: { print(stream, "%f", data->d.f); } break;
            case CT_TEXT: { 
                SString text = StringGet(handler->str, data->d.index);
                warn("text: %s", text);
                print(stream, "%s", text); 
            } break;
            default: break;
        }
        fclose(stream);
    }

    if (fork()) {
        return;
    }

    //INFO(ELI): We have to close both stdout and stdin to prevent the
    //terminal window from being polluted by garbage.

    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    //Equivalent to the command: Terminal texteditor filename
    execlp((char*)handler->preferred_terminal, (char*)handler->preferred_terminal, handler->preferred_text_editor, filename, 0);
    err("Failed to Execute Terminal");

    //crash if somehow the exec call fails
    panic();
}

void readConfig(RenderHandler* handler){
    FILE * configFile = fopen(CONFIG_FILEPATH, "r");
    if (!configFile){
        //NOTE(ELI): This seems like an error at the moment since we aren't
        //detecting any terminal or text editors.
        err("No Config File Detected.");
        return;
    }
    char configLine[STRING_SIZE];
    configLine[STRING_SIZE - 1] = '\0';
    // parse each line
    while (fgets(configLine, sizeof(configLine)-1, configFile) != NULL){
        strtok(configLine, " ");
        char * info = strtok(NULL, "\n");
        // configLine is the item, info is the content
        // oof no switch statements with strings (reasonable) :/
        //
        // NOTE(ELI): Yeah, technically switch statements are table
        // lookups (jump tables) so none of that unfortunately.

        log("info: %n %n", configLine, info);
        if (!strcmp(configLine, "terminal")){
            log("terminal line");
            strcpy((char*)handler->preferred_terminal, info);
            log("pt: %n", handler->preferred_terminal);
        }
        else if (!strcmp(configLine, "editor")){
            log("editor line");
            strcpy((char*)handler->preferred_text_editor, info);
            log("te: %n", handler->preferred_text_editor);
        }
        else if (!strcmp(configLine, "keypreset")) {
            log("key preset");
            if (!strcmp(info, "wasd")) {
                handler->keybinds = keybinds_wasd;
            } 
            else if (!strcmp(info, "hjkl")) {
                handler->keybinds = keybinds_hjkl;
            }
        }
        else {
            err("Invalid identifier in config file: %n", configLine);
        }
    }

    fclose(configFile);
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
            } 
            else if (keyIn == handler->keybinds.exit) {
                handler->state = SHUTDOWN;
            }
            else if (keyIn == handler->keybinds.terminal) {
                handler->state = TERMINAL;
            }
            else if (keyIn == handler->keybinds.edit) {
                handler->state = EDIT;
                //NOTE(ELI): Handler is already a pointer and doesn't need to have its address taken
                //This was the main bug in the previous version, it was reinterpreting a weird part of
                //the stack as a RenderHandler and reading garbage values.
                editCell(handler);
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

    // Set Cursor and Spread Sheet View

    //INFO(ELI): The 1 and 2 are derived from the values to
    //garuntee the cursor never goes partially offscreen.
    //
    //1 is essentially like a floor
    //2 comes from preventing partial off screen and 1 from the statusline
    i32 maxwidth = ((COLS - MARGIN_LEFT) / CELL_WIDTH) - 1;
    i32 maxheight = ((LINES - MARGIN_TOP) / CELL_HEIGHT) - 2;

    if (handler->cursor.x > maxwidth) handler->base.x++;
    if (handler->cursor.y > maxheight) handler->base.y++;
    if (handler->cursor.x < 0) handler->base.x--;
    if (handler->cursor.y < 0) handler->base.y--;

    
    handler->base.x = MAX(handler->base.x, 0);
    handler->base.y = MAX(handler->base.y, 0);

    handler->cursor.x = CLAMP(handler->cursor.x, 0, maxwidth);
    handler->cursor.y = CLAMP(handler->cursor.y, 0, maxheight);
}


int main(int argc, char* argv[]) {
    // ncurses setup
    initscr();
    noecho();               //prevent echoing output
    raw();                  //remove buffering
    keypad(stdscr, TRUE);   //enable extended keys
    set_escdelay(0);        //remove delay when processing esc
    curs_set(0);            //set cursor to be invisible

    //set logging
    //INFO(ELI): dev/null stops all printing,
    //change to enable printing to log file
    //logfile = fopen("/dev/null", "w+");
    logfile = fopen("log.out", "w+");
    errfile = fopen("err.out", "w+");

    StringTable str = (StringTable) {
        .mem = GlobalAllocatorCreate(),
    };

    //actual spreadsheet
    SpreadSheet sheet = (SpreadSheet){
        .mem = GlobalAllocatorCreate(),
    };

    //file descriptor table
    SpreadSheet files = (SpreadSheet){
        .mem = GlobalAllocatorCreate(),
    };

    if (argc >= 2) {
        FILE* csv = fopen(argv[1], "r"); 
        if (csv) {
            csv_load_file(csv, &str, &sheet);
        }
    }

    // if you want different keybinds u change that here
    RenderHandler handler = {
        .ch = 0,
        .base = {0, 0},
        .cursor = {0, 0},
        .state = NORMAL,
        .sheet = &sheet,
        .files = &files,
        .str = &str,
        .keybinds = keybinds_hjkl,
        //These are zero initialized if not mentioned,
        //This works for fixed sized structs
        //
        //Also side note, "" string literals are of type
        //char*, which means they must be assigned to a pointer.
        //Easy mistake to make but they have static memory allocated
        //at program startup rather than dynamically on the stack
    };
    
    readConfig(&handler);
    log("term: %n", handler.preferred_terminal);
    log("edit: %n", handler.preferred_text_editor);

    


    // rendering loop
    while (1) {
        erase();
        //rendering

        //Top numbering
        for (u32 i = 0; i < (COLS - MARGIN_LEFT)/CELL_WIDTH + 1; i++) {
            mvprintw(0, (i * CELL_WIDTH) + (CELL_WIDTH/2) + MARGIN_LEFT, "%d", handler.base.x + i);
        }

        //Left Numbering
        for (u32 i = 0; i < (LINES - MARGIN_TOP)/CELL_HEIGHT + 1; i++) {
            mvprintw( (i * CELL_HEIGHT) + (CELL_HEIGHT/2 + MARGIN_TOP), 0, "%d", handler.base.y + i);
        }

        //Sheet

        //NOTE(ELI): +1 ensures that the spreadsheet goes off the edge of the screen
        //rather than leaving a blank row/column
        for (u32 i = 0; i < (COLS - MARGIN_LEFT)/CELL_WIDTH + 1; i++) {
            for (u32 j = 0; j < (LINES - MARGIN_TOP)/CELL_HEIGHT + 1; j++) {
                if (i == handler.cursor.x && j == handler.cursor.y) {
                    continue;
                }

                SString info = cellDisplay(&sheet, &str,
                                           (v2u){handler.base.x + i, handler.base.y + j},
                                           CELL_WIDTH - 2);

                drawBox((v2u){(i * CELL_WIDTH) + MARGIN_LEFT, (j * CELL_HEIGHT) + MARGIN_TOP}, 
                        (v2u){CELL_WIDTH, CELL_HEIGHT}, info);

            }
        }

        // currently selected cell
        attron(A_REVERSE);
        SString info = cellDisplay(&sheet, &str, (v2u)
                {handler.cursor.x + handler.base.x, handler.cursor.y + handler.base.y}, CELL_WIDTH - 2);

        drawBox((v2u){(handler.cursor.x) * CELL_WIDTH + MARGIN_LEFT, (handler.cursor.y) * CELL_HEIGHT + MARGIN_TOP},
                (v2u){CELL_WIDTH, CELL_HEIGHT}, info);


        mvprintw(LINES - 2, 0, "Cursor (%d %d)  ch: %d State: %s", 
                handler.cursor.x + handler.base.x, handler.cursor.y + handler.base.y, handler.ch, stateToString(handler.state).data);
        refresh();

        //updating
        handler.ch = getch();
        handleKey(&handler);
        if (handler.state == SHUTDOWN) break;

    }


    endwin();
    return 0;
}

SString cellDisplay(SpreadSheet* sheet, StringTable* str, v2u pos, u32 maxlen) {
    static i8 buf[CELL_WIDTH + 1] = {0};
    CellValue* cell = SpreadSheetGetCell(sheet, pos);

    if (!cell) {
        return (SString){NULL, 0};
    }

    SString value = {NULL, 0};
    memset(buf, 0, sizeof(buf));
    value.data = buf;
    char* fmt = "";

    switch (cell->t) {
        case CT_EMPTY: break;
        case CT_FLOAT: { fmt = "%f"; } break;
        case CT_INT: { fmt = "%d"; } break;
        case CT_TEXT: {  
            SString data = StringGet(str, cell->d.index);
            value.size = snprintf((char*)value.data, maxlen, "%.*s", data.size, data.data);
            return value;
        } break;

        default: { 
            endwin();
            todo();
        } break;
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
