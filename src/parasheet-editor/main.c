#include "libparasheet/lib_internal.h"
#include "libparasheet/csv.h"
#include "libparasheet/evaluator.h"
#include <asm-generic/errno-base.h>
#include <errno.h>
#include <linux/limits.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <util/util.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <ncurses.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/inotify.h>
#include <sys/select.h>


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

#define FPS_CAP 15


void drawBox(v2u pos, v2u size, SString str);

// supplies the currently selected keybinds
typedef struct KeyBinds {
    u32 cursor_up;
    u32 cursor_down;
    u32 cursor_left;
    u32 cursor_right;
    u32 terminal;
    u32 exit; // quit the program
    u32 edit; // open a cell for editing
    u32 nav;  // go back to normal mode
	u32 run_cell;
} KeyBinds;

typedef struct TypeBuffer {
    u32 top;
    i8 buf[CELL_WIDTH * 10 + 1];
} TypeBuffer;

// store a file reference with position of data
typedef struct EditHandles {
    u32 notify; //inotify context handle
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
    .nav = (u8)KEY_ESCAPE,
	.run_cell = 'r'
};
KeyBinds keybinds_wasd = {
    .cursor_up = 'w',
    .cursor_down = 's',
    .cursor_left = 'a',
    .cursor_right = 'd',
    .terminal = ':',
    .exit = 'q',
    .edit = (u8)KEY_ENTER_REAL,
    .nav = (u8)KEY_ESCAPE,
	.run_cell = 'r'
};

KeyBinds keybinds_arrows = {
    .cursor_up = KEY_UP,
    .cursor_down = KEY_DOWN,
    .cursor_left = KEY_LEFT,
    .cursor_right = KEY_RIGHT,
    .terminal = ':',
    .exit = 'q',
    .edit = (u8)KEY_ENTER_REAL,
    .nav = (u8)KEY_ESCAPE,
	.run_cell = 'r'
};

typedef enum EditorState {
    NORMAL,
    TERMINAL,
    EDIT,
    SHUTDOWN
} EditorState;

typedef struct RenderHandler {
    Allocator mem;
    u32 ch;
    v2i base;
    v2i cursor;
    EditHandles edit;
    EditorState state;
    SpreadSheet* srcSheet;
    SpreadSheet* dispSheet;
    StringTable* str;
    KeyBinds keybinds;
    TypeBuffer type;
    u8 sheetname[PATH_MAX + 1];
    u8 preferred_terminal[STRING_SIZE];
    u8 preferred_text_editor[STRING_SIZE];
} RenderHandler;

SString cellDisplay(RenderHandler* handle, v2u pos, u32 maxlen);

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

    CellValue* data = SpreadSheetGetCell(handler->srcSheet, pos);

    char filename[PATH_MAX + 1] = {0};
    snprintf(filename, PATH_MAX, "./cells/cell_%d_%d", pos.x, pos.y);
    //snprintf(filename, PATH_MAX, "test");

    if (data) {
        FILE* stream = fopen(filename, "w");
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

    close(STDERR_FILENO);
    close(STDIN_FILENO);
    close(STDOUT_FILENO);

    char cmd[PATH_MAX + 1] = {0};
    snprintf(cmd, PATH_MAX, (char*)handler->preferred_text_editor, filename);
    system(cmd);

    exit(0);

}

void runCommand(RenderHandler* hand) {
    SString trimmed = {.data = hand->type.buf, .size = hand->type.top};

    while (trimmed.data[0] == ' ') {
        trimmed.data++;
        trimmed.size--;
    }

    while (trimmed.data[trimmed.size - 1] == ' ') {
        trimmed.size--;
    }

    
    log("command: \"%s\"", trimmed);
    if (SStrCmp(trimmed, sstring("save")) == 0) {
        if (hand->sheetname[0]) csv_export_file(hand->sheetname, hand->srcSheet, hand->str);
    }

    SString export = sstring("export");
    if ((trimmed.size > export.size + 1) && (memcmp(trimmed.data, export.data, export.size) == 0)) {
        SString name = (SString){.data = trimmed.data, .size = trimmed.size};
        while ((name.data - trimmed.data < trimmed.size) && name.data[0] != ' '){
            name.data++;
            name.size--;
        }
        name.data++;
        name.size--;

        log("export \"%s\"", name);
        csv_export_file((char*)name.data, hand->dispSheet, hand->str);
    }

    SString rename = sstring("rename");
    if ((trimmed.size > rename.size + 1) && (memcmp(trimmed.data, rename.data, rename.size) == 0)) {
        
        SString name = (SString){.data = trimmed.data, .size = trimmed.size};
        while ((name.data - trimmed.data < trimmed.size) && name.data[0] != ' '){
            name.data++;
            name.size--;
        }
        name.data++;
        name.size--;
        log("rename %s", name);
        strncpy(hand->sheetname, name.data, PATH_MAX);
    }


    SString open = sstring("open");
    if ((trimmed.size > open.size + 1) && (memcmp(trimmed.data, open.data, open.size) == 0)) {
        SString name = (SString){.data = trimmed.data, .size = trimmed.size};
        while ((name.data - trimmed.data < trimmed.size) && name.data[0] != ' '){
            name.data++;
            name.size--;
        }
        name.data++;
        name.size--;
        log("open \"%s\"", name.data);

        FILE* csv = fopen((char*)name.data, "r");
        log("file: %p", csv);
        if (!csv) {
            err("Failed to load file: %n",strerror(errno));
            return;
        }

        SpreadSheetClear(hand->srcSheet);
        SpreadSheetClear(hand->dispSheet);

        csv_load_file(csv, hand->str, hand->srcSheet);
        strncpy(hand->sheetname, name.data, PATH_MAX);
    }

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
        if (!strcmp(configLine, "editor")){
            log("editor line");
            strcpy((char*)handler->preferred_text_editor, info);
            log("te: \"%n\"", handler->preferred_text_editor);
        }
        else if (!strcmp(configLine, "keypreset")) {
            log("key preset");
            if (!strcmp(info, "wasd")) {
                handler->keybinds = keybinds_wasd;
            } else if (!strcmp(info, "hjkl")) {
                handler->keybinds = keybinds_hjkl;
            } else if (!strcmp(info, "arrow")) {
                handler->keybinds = keybinds_arrows;
            }
        }
        else {
            err("Invalid identifier in config file: %n", configLine);
        }
    }

    fclose(configFile);
}

void handleKey(RenderHandler* handler){

    u32 keyIn = handler->ch;
    switch (handler->state){
        case NORMAL:
            if (keyIn == handler->keybinds.cursor_up) {
                handler->cursor.y -= 1;
            } else if (keyIn == handler->keybinds.cursor_down) {
                handler->cursor.y += 1;
            } else if (keyIn == handler->keybinds.cursor_left) {
                handler->cursor.x -= 1;
            } else if (keyIn == handler->keybinds.cursor_right) {
                handler->cursor.x += 1;
            } else if (keyIn == handler->keybinds.exit) {
                handler->state = SHUTDOWN;
            } else if (keyIn == handler->keybinds.terminal) {
                handler->state = TERMINAL;
                memset(handler->type.buf, 0, CELL_WIDTH * 10 + 1); 
                handler->type.top = 0;
            } else if (keyIn == handler->keybinds.edit) {
                handler->state = EDIT;
                //NOTE(ELI): Handler is already a pointer and doesn't need to have its address taken
                //This was the main bug in the previous version, it was reinterpreting a weird part of
                //the stack as a RenderHandler and reading garbage values.
                editCell(handler);
				handler->state = NORMAL;
            } else if (keyIn == handler->keybinds.run_cell){

                //TODO(ELI): I will implement a ClearSheet function,
                //that way we can clear a spreadsheet without freeing
                //the memory.
                //
                //Also we should only need two sheets. All writes go to
                //display sheet, and anything not written can default
                //to src sheet in the cell display function. Basically
                //Display sheet acts like an overlay rather than being
                //the current display sheet directly.

                SymbolTable sym = {
                    .mem = GlobalAllocatorCreate(),
                };
                SymbolPushScope(&sym);

				EvaluateCell(
					(EvalContext){
                        .mem = GlobalAllocatorCreate(),
						.srcSheet = handler->srcSheet,
						.inSheet = handler->dispSheet,
						.outSheet = handler->dispSheet,
                        .str = handler->str,
                        .table = &sym,
						.currentX = handler->cursor.x,
						.currentY = handler->cursor.y
					}
				);

                while (sym.size) {
                    SymbolPopScope(&sym);
                }
                Free(sym.mem, sym.scopes, sym.cap * sizeof(SymbolMap));

				// copies values from tempSheet to dispSheet after execution is done
				//CellValue* transfer;
				//for (int i = 0; i < tempSheet.size; i++){
				//	transfer = SpreadSheetGetCell(&tempSheet, tempSheet.keys[i]);
				//	SpreadSheetSetCell(handler->dispSheet, tempSheet.keys[i], *transfer);
				//}
				//SpreadSheetFree(&tempSheet);
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
            } else if (keyIn == KEY_BACKSPACE && handler->type.top) {
                handler->type.buf[--handler->type.top] = 0;
            } else if (keyIn == KEY_ENTER_REAL) {
                runCommand(handler);
                handler->state = NORMAL;
            } else {
                if (handler->type.top > CELL_WIDTH * 2) break;
                handler->type.buf[handler->type.top++] = keyIn;
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

void ReadBuffer(RenderHandler* hand, SString name) {
    if (name.size < 6 || memcmp(name.data, "cell", 4) != 0) {
        return;
    }

    char bufname[PATH_MAX + 1] = {0};
    snprintf(bufname, PATH_MAX, "./cells/%.*s", name.size, name.data);
    log("buf: %n", bufname);

    FILE* buffer = fopen(bufname, "r");
    if (!buffer) {
        return;
    }
    
    struct stat info;
    if (stat(bufname, &info)) {
        err("Failed to Stat");
    }

    u32 xstr = 0;
    while (bufname[xstr] != '_') xstr++;

    u32 ystr = ++xstr;
    while (bufname[ystr] != '_') ystr++;
    bufname[ystr++] = 0;

    u32 x = stou(&bufname[xstr]);
    u32 y = stou(&bufname[ystr]);

    log("pos: (%d %d)", x, y);

    char* data = Alloc(hand->mem, info.st_size + 1);
    data[info.st_size] = 0;
    fread(data, info.st_size, 1, buffer);
    fclose(buffer);

    if (info.st_size) {
        u32 end = info.st_size - 1;
        while (isspace(data[end])) { 
            info.st_size--;
            end--;
        }
        data[end + 1] = 0;
    }


    print(logfile, "%s\n", (SString){.data = (i8*)data, .size = info.st_size});

    CellValue* c = SpreadSheetGetCell(hand->srcSheet, (v2u){x,y});
    if (c && c->t == CT_TEXT) {
        SString text = StringGet(hand->str, c->d.index);
        Free(hand->mem, text.data, text.size); 
    }

    CellValue new = {0};

    if (info.st_size == 0) {
        new.t = CT_EMPTY;
    } else if (is_integer(data)) {
        new.t = CT_INT;
        new.d.i = atoi(data);
        Free(hand->mem, data, info.st_size);
    } else if (is_float(data)) {
        new.t = CT_FLOAT;
        new.d.f = atof(data);
        Free(hand->mem, data, info.st_size);
    } else {
        new.t = CT_TEXT;
        new.d.index = StringAdd(hand->str, (i8*)data);
    }

    SpreadSheetSetCell(hand->srcSheet, (v2u){x, y}, new);
    SpreadSheetClearCell(hand->dispSheet, (v2u){x, y});
}


int main(int argc, char* argv[]) {
    // ncurses setup
    initscr();
    noecho();               //prevent echoing output
    raw();                  //remove buffering
    nl();                   //disable newline
    keypad(stdscr, TRUE);   //enable extended keys
    set_escdelay(0);        //remove delay when processing esc
    curs_set(0);            //set cursor to be invisible

    //set logging
    //INFO(ELI): dev/null stops all printing,
    //change to enable printing to log file
    //logfile = fopen("/dev/null", "w+");
    logfile = fopen("log.out", "w+");
    errfile = logfile;

    StringTable str = (StringTable) {
        .mem = GlobalAllocatorCreate(),
    };

    // actual spreadsheet
	// spreadsheet with true values
    SpreadSheet srcSheet = (SpreadSheet){
        .mem = GlobalAllocatorCreate(),
    };
    SpreadSheet dispSheet = (SpreadSheet){
        .mem = GlobalAllocatorCreate(),
    };

    
    RenderHandler handler = {
        .mem = GlobalAllocatorCreate(),
        .ch = 0,
        .base = {0, 0},
        .cursor = {0, 0},
        .state = NORMAL,
        .srcSheet = &srcSheet,
        .dispSheet = &dispSheet,
        .str = &str,
        .keybinds = keybinds_hjkl,
        .edit = {
            .notify = inotify_init1(IN_NONBLOCK),
        },
        //These are zero initialized if not mentioned,
        //This works for fixed sized structs
        //
        //Also side note, "" string literals are of type
        //char*, which means they must be assigned to a pointer.
        //Easy mistake to make but they have static memory allocated
        //at program startup rather than dynamically on the stack
    };
    log("notify: %d", handler.edit.notify);

    if (rmdir("cells")) {
        log("rmdir: %n", strerror(errno));
    }
    if (mkdir("./cells", 0777)) {
        log("mkdir: %n", strerror(errno));
    }

    u32 t = inotify_add_watch(handler.edit.notify, "./cells", IN_CREATE | IN_MODIFY | IN_ONLYDIR & ~IN_ACCESS);
    if (t == -1) {
        log("watch: %n", strerror(errno));
    }

    
    if (argc >= 2) {
        FILE* csv = fopen(argv[1], "r"); 
        if (csv) {
            csv_load_file(csv, &str, &srcSheet);
            csv_load_file(csv, &str, &dispSheet);
            strncpy(handler.sheetname, argv[0], PATH_MAX);
        }
    }

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

                SString info = cellDisplay(&handler,
                                           (v2u){handler.base.x + i, handler.base.y + j},
                                           CELL_WIDTH - 2);

                drawBox((v2u){(i * CELL_WIDTH) + MARGIN_LEFT, (j * CELL_HEIGHT) + MARGIN_TOP}, 
                        (v2u){CELL_WIDTH, CELL_HEIGHT}, info);

            }
        }

        // currently selected cell
        attron(A_REVERSE);
        SString info = cellDisplay(&handler, (v2u)
                {handler.cursor.x + handler.base.x, handler.cursor.y + handler.base.y}, CELL_WIDTH - 2);

        drawBox((v2u){(handler.cursor.x) * CELL_WIDTH + MARGIN_LEFT, (handler.cursor.y) * CELL_HEIGHT + MARGIN_TOP},
                (v2u){CELL_WIDTH, CELL_HEIGHT}, info);


        mvprintw(LINES - 2, MARGIN_LEFT + 2, "%s Cursor (%d %d)  ch: %d State: %s", handler.sheetname, 
                handler.cursor.x + handler.base.x, handler.cursor.y + handler.base.y, handler.ch, stateToString(handler.state).data);
        if (handler.state == TERMINAL) mvprintw(LINES - 3, MARGIN_LEFT + 2, ":%.*s", handler.type.top, handler.type.buf);
        refresh();
		usleep(25600);

        //updating
        
        struct pollfd fd[2] = {0};
        fd[0] = (struct pollfd) {
            .fd = 0,
            .events = POLLIN,
        };

        fd[1] = (struct pollfd) {
            .fd = handler.edit.notify,
            .events = POLLIN
        };

        u32 found = poll(fd, 2, -1);

        if (!found) {
            continue;
        }

        if (fd[0].revents & POLLIN) {
            handler.ch = getch();
            handleKey(&handler);
            if (handler.state == SHUTDOWN) break;
            continue;
        }

        if (fd[1].revents & POLLIN) {
            i64 r = 0;
            do {
                struct inotify_event events[256] = {0};
                r = read(fd[1].fd, events, (256 * sizeof(struct inotify_event)));

                struct inotify_event* event = (struct inotify_event*)&events[0];

                while (event < (&events[256])) {
                    ReadBuffer(&handler, (SString){(i8*)event->name, event->len});
                    event += sizeof(struct inotify_event) + event->len;
                }

            } while (r > 0);
            continue;
        }

    }


    endwin();
    return 0;
}

SString cellDisplay(RenderHandler* handle, v2u pos, u32 maxlen) {
    static i8 buf[CELL_WIDTH + 2] = {0};

    //prefer display sheet, src sheet as fallback
    CellValue* cell = SpreadSheetGetCell(handle->dispSheet, pos);
    if (!cell || cell->t == CT_EMPTY) cell = SpreadSheetGetCell(handle->srcSheet, pos);
    if (!cell) {
        return (SString){NULL, 0};
    }

    SString value = {NULL, 0};
    memset(buf, 0, sizeof(buf));
    value.data = buf;

    switch (cell->t) {
        case CT_EMPTY: break;
        case CT_FLOAT: { 
            value.size = snprintf((char*)value.data, CELL_WIDTH + 1, "%f", cell->d.f);
        } break;
        case CT_INT: { 
            value.size = snprintf((char*)value.data, CELL_WIDTH + 1, "%d", cell->d.i);
        } break;
        case CT_TEXT: {  
            SString data = StringGet(handle->str, cell->d.index);
            for (u32 i = 0; i < MIN(maxlen, data.size); i++) {
                if (data.data[i] == '\n' || data.data[i] == '\t') break;
                value.data[i] = data.data[i];
                value.size++;
            }
            return value;
        } break;

        default: { 
            endwin();
            todo();
        } break;
    }

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
