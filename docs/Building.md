# Building this project
This project uses GNU make as a build system. To build, you need to be on a supported operating system (OSX or Linux), have all necessary libraries installed, have clang, and have make. To build, simply use make, and specify the target that you would like to build.

There are a variety of extra non-file targets defined in the Makefile for your convienince. The full list can be viewed by running `make help`:
```
GNU make is a build system that builds targets, which are usually files. GNU make looks at dependencies and file update timestamps so it will only rebuild when necessary. The following extra non-file build targets have been defined in the Makefile for this project:

all - Build all targets
clean - Delete all build outputs
help - Display this help page
all-debug - Build the all debug targets
all-release - Build all release targets
editor-all - Build all parasheet-editor targets
editor-debug - Build all parasheet-editor debug targets
editor-release - Build all parasheet-editor release targets
editor-run - Build and run the debug version of parasheet-editor
editor-run-release - Build and run the debug version of parasheet-editor
cli-all - Build all parasheet-cli targets
cli-debug - Build all parasheet-cli debug targets
cli-release - Build all parasheet-cli release targets
cli-run - Build and run the debug version of parasheet-cli
cli-run-release - Build and run the debug version of parasheet-cli
library-all - Build all libparasheet targets
library-debug - Build all libparasheet debug targets
library-release - Build all libparasheet release targets
util-all - Build all util targets
util-debug - Build all util debug targets
util-release - Build all util release targets
```
