# Building this project
This project uses GNU make as a build system. To build, you need to be on a supported operating system (OSX or Linux), have all necessary libraries installed, have clang, and have make. To build, simply use make, and specify the target that you would like to build.

There are a variety of extra non-file targets defined in the Makefile for your convienince. The full list can be viewed by running `make help`:
```
GNU make is a build system that builds targets, which are usually files. GNU make looks at dependencies and file update timestamps so it will only rebuild when necessary. The following extra non-file build targets have been defined in the Makefile for this project:

all - Build all targets
clean - Delete all build outputs
help - Display this help page
all-debug - Build the all debug targets
all-debug-notests - Build the all debug executables without building tests
all-release - Build all release targets
all-build-tests - Build all tests without running them
all-test - Build and run all tests
editor-all - Build all parasheet-editor targets
editor-debug - Build all parasheet-editor debug targets
editor-debug-notests - Build the parasheet-editor debug executable without building tests
editor-release - Build all parasheet-editor release targets
editor-build-tests - Build parasheet-editor tests without running them
editor-test - Build and run all parasheet-editor tests
editor-run - Build and run the debug version of parasheet-editor
editor-run-valgrind - Build and run the debug version of parasheet-editor with valgrind
editor-run-release - Build and run the debug version of parasheet-editor
cli-all - Build all parasheet-cli targets
cli-debug - Build all parasheet-cli debug targets
cli-debug-notests - Build the parasheet-cli debug executable without building tests
cli-release - Build all parasheet-cli release targets
cli-build-tests - Build parasheet-cli tests without running them
cli-test - Build and run all parasheet-cli tests
cli-run - Build and run the debug version of parasheet-cli
cli-run-valgrind - Build and run the debug version of parasheet-cli with valgrind
cli-run-release - Build and run the debug version of parasheet-cli
library-all - Build all libparasheet targets
library-debug - Build all libparasheet debug targets
library-debug-notests - Build debug libparasheet without building tests
library-release - Build all libparasheet release targets
library-build-tests - Build libparasheet tests without running them
library-test - Build and run all libparasheet tests
util-all - Build all util targets
util-debug - Build all util debug targets
util-debug-notests - Build all util debug object files without building tests
util-release - Build all util release targets
util-build-tests - Build util tests without running them
util-test - Build and run all util tests

For any of the run or test targets, you cat set the ARGS environment variable to pass in arguments. When using valgrind, you can set the VALGRIND_ARGS environment variable to pass in arguments to valgrind.
```
