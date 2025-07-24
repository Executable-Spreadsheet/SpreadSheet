# Parasheet

## Installation Guide
Clone this repository from the main branch.
Edit the configuration file, called `config.txt`, in the main directory.
Then, run "`make editor-run`" in your terminal of choice.

## Config file

The config file looks like this:
```
editor
keybind
```

After editor, write the command used to open a new terminal window and open a text editor. The %s is where the filename would be. It should be separated from the word editor by a space. Be careful when writing this command, as we do not validate the command for you, and it does run in your terminal.

After keybind, write the keybind you would prefer. The options are hjkl, wasd, and arrow.

## Usage
The editor has multiple modes:
Press : to enter terminal mode. Press ESC to return to normal mode.
In normal mode, 'r' will run the current cell. 'ENTER' will edit the currently selected cell using the command specified in the config file. 'q' will close the editor.

There are also terminal commands accessed by typing the following:

`:open <filename>` loads a csv
`:rename <new name>` renames the current sheet
`:export <filename>` exports the display sheet
`:save` saves current srcsheet to a csv

You can see the current command in the bottom left above the status line.


Updated: today


## Decisions
- Language : C


## User Stories
- Create Spreadsheet
    - Save to file?
- Edit Spreadsheet
    - Display a Spreadsheet
    - Replace a cell's contents
    - Name A Cell
    - Edit cell in text editor
- Run Spreadsheet
    - Execute contents of a cell
    - Display executed cells
- Load a Spreadsheet
    - load a CSV

- Use app on Linux/Unix
- Use app on Windows (Low Priority)

## Spikes
- Explore Spreadsheet Data Structures
    - Requirement: Sparse, Growable, multidimensional

- Language Grammar
    - Functional
    - Simple

- Architecture Design
    - Performant
    - Easy to solve Circular Dependencies

## Infrastructure

- Target Platform
    - UNIX
    - x86/arm

- Test Suite
    - Note: We can use llvm analysis tools

- Build System
    - Compiler: clang
    - Build Tool: make

- CI/CD (testing/build automation)

