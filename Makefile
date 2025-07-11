# Define the compiler to use
CC:=clang
# Define the source file extension (This is used to detect which files to compile)
SOURCE_FILE_EXTENSION:=.c
# Define compiler flags used in all builds
CFLAGS:=-Wall
# Define linker flags (These are still passed into the main compiler command, just after the -o)
LDFLAGS:=

# Define compiler flags used in debug & release builds
DEBUG_CFLAGS:=-g -D DEBUG
RELEASE_CFLAGS:=-O3

# Define base directories
SRC_DIR:=src
INCLUDE_DIRS:=include
TEST_SRC_DIR:=tests
BUILD_DIR:=build

# Platform specific things go here
# Detection of platform and archetecture are from https://stackoverflow.com/a/12099167
ifeq ($(OS),Windows_NT)
	CFLAGS+= -D WIN32
	LDFLAGS+=

	osexefiles=$(addsuffix .exe,$1)
	ossharedlibfiles=$(addsuffix .dll,$1) # Function to convert library files to the format used by the OS (most of the time)
	ospath=$(subst /,\,$1) # Function to convert file paths to the format used by the OS
	MKDIR=mkdir $(call ospath,$@)
	CLEAN=$(if $(wildcard $(BUILD_DIR)), rmdir /Q /S $(wildcard $(BUILD_DIR)))

	ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
		CFLAGS+= -D AMD64 -m64 # Here the extra -m64 flag tells the compiler to compile for 64 bit
	else
		ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
			CFLAGS+= -D AMD64 -m64 # Here the extra -m64 flag tells the compiler to compile for 64 bit
		endif
		ifeq ($(PROCESSOR_ARCHITECTURE),x86)
			CFLAGS+= -D IA32
		endif
	endif
else
	LDFLAGS+=

	osexefiles=$1
	ossharedlibfiles=$(foreach LIBFILE,$1,lib$(notdir $(LIBFILE)).so) # Function to convert library files to the format used by the OS
	ospath=$1 # Function to convert file paths to the format used by the OS
	MKDIR=mkdir -p $(call ospath,$@)
	# Here the if function is used to prevent an error caused by attempting to deleting a file that does not exist
	CLEAN=$(if $(wildcard $(BUILD_DIR)), rm -rf $(wildcard $(BUILD_DIR)))

	UNAME_S:=$(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		CFLAGS+= -D LINUX
	endif
	ifeq ($(UNAME_S),Darwin)
		CFLAGS+= -D OSX
		LDFLAGS+=
	endif
	UNAME_P:=$(shell uname -p)
	ifeq ($(UNAME_P),x86_64)
		CFLAGS+= -D AMD64 -m64 # Here the extra -m64 flag tells the compiler to compile for 64 bit
	endif
	ifneq ($(filter %86,$(UNAME_P)),)
		CFLAGS+= -D IA32
	endif
	ifneq ($(filter arm%,$(UNAME_P)),)
		CFLAGS+= -D ARM
	endif
endif

# parasheet-editor directories
EDITOR_SRC_DIR:=$(SRC_DIR)/parasheet-editor
EDITOR_INCLUDE_DIRS:=$(INCLUDE_DIRS)
EDITOR_TEST_SRC_DIR:=$(TEST_SRC_DIR)/parasheet-editor
EDITOR_BUILD_DIR:=$(BUILD_DIR)/parasheet-editor
EDITOR_CFLAGS:=$(CFLAGS)
EDITOR_LDFLAGS:=$(LDFLAGS) -lparasheet
EDITOR_RELEASE_DIR:=$(EDITOR_BUILD_DIR)/release
EDITOR_RELEASE_OBJ_DIR:=$(EDITOR_RELEASE_DIR)/objs
EDITOR_RELEASE_DEP_DIR:=$(EDITOR_RELEASE_DIR)/deps
EDITOR_RELEASE_EXE:=$(EDITOR_RELEASE_DIR)/$(call osexefiles,parasheet-editor)
EDITOR_RELEASE_CFLAGS:=$(EDITOR_CFLAGS) $(RELEASE_CFLAGS)
EDITOR_RELEASE_LDFLAGS=$(EDITOR_LDFLAGS) -L$(LIBRARY_RELEASE_DIR)
EDITOR_DEBUG_DIR:=$(EDITOR_BUILD_DIR)/debug
EDITOR_DEBUG_OBJ_DIR:=$(EDITOR_DEBUG_DIR)/objs
EDITOR_DEBUG_DEP_DIR:=$(EDITOR_DEBUG_DIR)/deps
EDITOR_DEBUG_EXE:=$(EDITOR_DEBUG_DIR)/$(call osexefiles,parasheet-editor)
EDITOR_DEBUG_CFLAGS:=$(EDITOR_CFLAGS) $(DEBUG_CFLAGS)
EDITOR_DEBUG_LDFLAGS=$(EDITOR_LDFLAGS) -L$(LIBRARY_DEBUG_DIR)
EDITOR_TEST_DIR:=$(EDITOR_DEBUG_DIR)/tests
EDITOR_TEST_EXE_DIR:=$(EDITOR_TEST_DIR)/exes
EDITOR_TEST_DEP_DIR:=$(EDITOR_TEST_DIR)/deps
EDITOR_TEST_CFLAGS:=$(EDITOR_CFLAGS) $(DEBUG_CFLAGS)
EDITOR_TEST_LDFLAGS=$(EDITOR_LDFLAGS) -L$(LIBRARY_DEBUG_DIR)

# parasheet-cli directories
CLI_SRC_DIR:=$(SRC_DIR)/parasheet-cli
CLI_INCLUDE_DIRS:=$(INCLUDE_DIRS)
CLI_TEST_SRC_DIR:=$(TEST_SRC_DIR)/parasheet-cli
CLI_BUILD_DIR:=$(BUILD_DIR)/parasheet-cli
CLI_CFLAGS:=$(CFLAGS)
CLI_LDFLAGS:=$(LDFLAGS) -lparasheet
CLI_RELEASE_DIR:=$(CLI_BUILD_DIR)/release
CLI_RELEASE_OBJ_DIR:=$(CLI_RELEASE_DIR)/objs
CLI_RELEASE_DEP_DIR:=$(CLI_RELEASE_DIR)/deps
CLI_RELEASE_EXE:=$(CLI_RELEASE_DIR)/$(call osexefiles,parasheet-cli)
CLI_RELEASE_CFLAGS:=$(CLI_CFLAGS) $(RELEASE_CFLAGS)
CLI_RELEASE_LDFLAGS=$(CLI_LDFLAGS) -L$(LIBRARY_RELEASE_DIR)
CLI_DEBUG_DIR:=$(CLI_BUILD_DIR)/debug
CLI_DEBUG_OBJ_DIR:=$(CLI_DEBUG_DIR)/objs
CLI_DEBUG_DEP_DIR:=$(CLI_DEBUG_DIR)/deps
CLI_DEBUG_EXE:=$(CLI_DEBUG_DIR)/$(call osexefiles,parasheet-cli)
CLI_DEBUG_CFLAGS:=$(CLI_CFLAGS) $(DEBUG_CFLAGS)
CLI_DEBUG_LDFLAGS=$(CLI_LDFLAGS) -L$(LIBRARY_DEBUG_DIR)
CLI_TEST_DIR:=$(CLI_DEBUG_DIR)/tests
CLI_TEST_EXE_DIR:=$(CLI_TEST_DIR)/exes
CLI_TEST_DEP_DIR:=$(CLI_TEST_DIR)/deps
CLI_TEST_CFLAGS:=$(CLI_CFLAGS) $(DEBUG_CFLAGS)
CLI_TEST_LDFLAGS=$(CLI_LDFLAGS) -L$(LIBRARY_DEBUG_DIR)

# libparasheet directories
LIBRARY_SRC_DIR:=$(SRC_DIR)/libparasheet
LIBRARY_INCLUDE_DIRS:=$(INCLUDE_DIRS)
LIBRARY_TEST_SRC_DIR:=$(TEST_SRC_DIR)/libparasheet
LIBRARY_BUILD_DIR:=$(BUILD_DIR)/libparasheet
LIBRARY_CFLAGS:=$(CFLAGS) -fPIC
LIBRARY_LDFLAGS:=$(LDFLAGS)
LIBRARY_RELEASE_DIR:=$(LIBRARY_BUILD_DIR)/release
LIBRARY_RELEASE_OBJ_DIR:=$(LIBRARY_RELEASE_DIR)/objs
LIBRARY_RELEASE_DEP_DIR:=$(LIBRARY_RELEASE_DIR)/deps
LIBRARY_RELEASE_LIB:=$(LIBRARY_RELEASE_DIR)/$(call ossharedlibfiles,parasheet)
LIBRARY_RELEASE_CFLAGS:=$(LIBRARY_CFLAGS) $(RELEASE_CFLAGS)
LIBRARY_RELEASE_LDFLAGS:=$(LIBRARY_LDFLAGS)
LIBRARY_DEBUG_DIR:=$(LIBRARY_BUILD_DIR)/debug
LIBRARY_DEBUG_OBJ_DIR:=$(LIBRARY_DEBUG_DIR)/objs
LIBRARY_DEBUG_DEP_DIR:=$(LIBRARY_DEBUG_DIR)/deps
LIBRARY_DEBUG_LIB:=$(LIBRARY_DEBUG_DIR)/$(call ossharedlibfiles,parasheet)
LIBRARY_DEBUG_CFLAGS:=$(LIBRARY_CFLAGS) $(DEBUG_CFLAGS)
LIBRARY_DEBUG_LDFLAGS:=$(LIBRARY_LDFLAGS)
LIBRARY_TEST_DIR:=$(LIBRARY_DEBUG_DIR)/tests
LIBRARY_TEST_EXE_DIR:=$(LIBRARY_TEST_DIR)/exes
LIBRARY_TEST_DEP_DIR:=$(LIBRARY_TEST_DIR)/deps
LIBRARY_TEST_CFLAGS:=$(LIBRARY_CFLAGS) $(DEBUG_CFLAGS)
LIBRARY_TEST_LDFLAGS=$(LIBRARY_LDFLAGS)

# util directories
UTIL_SRC_DIR:=$(SRC_DIR)/util
UTIL_INCLUDE_DIRS:=$(INCLUDE_DIRS)
UTIL_TEST_SRC_DIR:=$(TEST_DIR)/util
UTIL_BUILD_DIR:=$(BUILD_DIR)/util
UTIL_CFLAGS:=$(CFLAGS) -fPIC
UTIL_LDFLAGS:=$(LDFLAGS)
UTIL_RELEASE_DIR:=$(UTIL_BUILD_DIR)/release
UTIL_RELEASE_OBJ_DIR:=$(UTIL_RELEASE_DIR)/objs
UTIL_RELEASE_DEP_DIR:=$(UTIL_RELEASE_DIR)/deps
UTIL_RELEASE_CFLAGS:=$(UTIL_CFLAGS) $(RELEASE_CFLAGS)
UTIL_RELEASE_LDFLAGS:=$(UTIL_LDFLAGS)
UTIL_DEBUG_DIR:=$(UTIL_BUILD_DIR)/debug
UTIL_DEBUG_OBJ_DIR:=$(UTIL_DEBUG_DIR)/objs
UTIL_DEBUG_DEP_DIR:=$(UTIL_DEBUG_DIR)/deps
UTIL_DEBUG_CFLAGS:=$(UTIL_CFLAGS) $(DEBUG_CFLAGS)
UTIL_DEBUG_LDFLAGS:=$(UTIL_LDFLAGS)
UTIL_TEST_DIR:=$(UTIL_DEBUG_DIR)/tests
UTIL_TEST_EXE_DIR:=$(UTIL_TEST_DIR)/exes
UTIL_TEST_DEP_DIR:=$(UTIL_TEST_DIR)/deps
UTIL_TEST_CFLAGS:=$(UTIL_CFLAGS) $(DEBUG_CFLAGS)
UTIL_TEST_LDFLAGS=$(UTIL_LDFLAGS)

# Define a recursive wildcard function to search for source files
# I got this function from https://stackoverflow.com/a/18258352
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2)$(filter $(subst *,%,$2),$d))

# Find source files using the recursive wildcard function
EDITOR_SRCS:=$(patsubst $(EDITOR_SRC_DIR)/%,%,$(call rwildcard,$(EDITOR_SRC_DIR),*$(SOURCE_FILE_EXTENSION)))
CLI_SRCS:=$(patsubst $(CLI_SRC_DIR)/%,%,$(call rwildcard,$(CLI_SRC_DIR),*$(SOURCE_FILE_EXTENSION)))
LIBRARY_SRCS:=$(patsubst $(LIBRARY_SRC_DIR)/%,%,$(call rwildcard,$(LIBRARY_SRC_DIR),*$(SOURCE_FILE_EXTENSION)))
UTIL_SRCS:=$(patsubst $(UTIL_SRC_DIR)/%,%,$(call rwildcard,$(UTIL_SRC_DIR),*$(SOURCE_FILE_EXTENSION)))

# Find test source files using the recursive wildcard function
EDITOR_TEST_SRCS:=$(patsubst $(EDITOR_TEST_SRC_DIR)/%,%,$(call rwildcard,$(EDITOR_TEST_SRC_DIR),*$(SOURCE_FILE_EXTENSION)))
CLI_TEST_SRCS:=$(patsubst $(CLI_TEST_SRC_DIR)/%,%,$(call rwildcard,$(CLI_TEST_SRC_DIR),*$(SOURCE_FILE_EXTENSION)))
LIBRARY_TEST_SRCS:=$(patsubst $(LIBRARY_TEST_SRC_DIR)/%,%,$(call rwildcard,$(LIBRARY_TEST_SRC_DIR),*$(SOURCE_FILE_EXTENSION)))
UTIL_TEST_SRCS:=$(patsubst $(UTIL_TEST_SRC_DIR)/%,%,$(call rwildcard,$(UTIL_TEST_SRC_DIR),*$(SOURCE_FILE_EXTENSION)))

# Create a list of object files to build
EDITOR_RELEASE_OBJS:=$(EDITOR_SRCS:%$(SOURCE_FILE_EXTENSION)=$(EDITOR_RELEASE_OBJ_DIR)/%.o)
EDITOR_DEBUG_OBJS:=$(EDITOR_SRCS:%$(SOURCE_FILE_EXTENSION)=$(EDITOR_DEBUG_OBJ_DIR)/%.o)
EDITOR_TEST_EXES:=$(EDITOR_TEST_SRCS:%$(SOURCE_FILE_EXTENSION)=$(EDITOR_TEST_EXE_DIR)/%)

CLI_RELEASE_OBJS:=$(CLI_SRCS:%$(SOURCE_FILE_EXTENSION)=$(CLI_RELEASE_OBJ_DIR)/%.o)
CLI_DEBUG_OBJS:=$(CLI_SRCS:%$(SOURCE_FILE_EXTENSION)=$(CLI_DEBUG_OBJ_DIR)/%.o)
CLI_TEST_EXES:=$(CLI_TEST_SRCS:%$(SOURCE_FILE_EXTENSION)=$(CLI_TEST_EXE_DIR)/%)

LIBRARY_RELEASE_OBJS:=$(LIBRARY_SRCS:%$(SOURCE_FILE_EXTENSION)=$(LIBRARY_RELEASE_OBJ_DIR)/%.o)
LIBRARY_DEBUG_OBJS:=$(LIBRARY_SRCS:%$(SOURCE_FILE_EXTENSION)=$(LIBRARY_DEBUG_OBJ_DIR)/%.o)
LIBRARY_TEST_EXES:=$(LIBRARY_TEST_SRCS:%$(SOURCE_FILE_EXTENSION)=$(LIBRARY_TEST_EXE_DIR)/%)

UTIL_RELEASE_OBJS:=$(UTIL_SRCS:%$(SOURCE_FILE_EXTENSION)=$(UTIL_RELEASE_OBJ_DIR)/%.o)
UTIL_DEBUG_OBJS:=$(UTIL_SRCS:%$(SOURCE_FILE_EXTENSION)=$(UTIL_DEBUG_OBJ_DIR)/%.o)
UTIL_TEST_EXES:=$(UTIL_TEST_SRCS:%$(SOURCE_FILE_EXTENSION)=$(UTIL_TEST_EXE_DIR)/%)

# Create a list of subdirectories so that the directory structure can be recreated in the object directory
EDITOR_RELEASE_SUBDIRECTORES:=$(sort $(patsubst %/,%,$(dir $(addprefix $(EDITOR_RELEASE_OBJ_DIR)/,$(EDITOR_SRCS))))) $(sort $(patsubst %/,%,$(dir $(addprefix $(EDITOR_RELEASE_DEP_DIR)/,$(EDITOR_SRCS)))))
EDITOR_DEBUG_SUBDIRECTORES:=$(sort $(patsubst %/,%,$(dir $(addprefix $(EDITOR_DEBUG_OBJ_DIR)/,$(EDITOR_SRCS))))) $(sort $(patsubst %/,%,$(dir $(addprefix $(EDITOR_DEBUG_DEP_DIR)/,$(EDITOR_SRCS)))))
EDITOR_TEST_SUBDIRECTORES:=$(sort $(patsubst %/,%,$(dir $(addprefix $(EDITOR_TEST_EXE_DIR)/,$(EDITOR_TEST_SRCS))))) $(sort $(patsubst %/,%,$(dir $(addprefix $(EDITOR_TEST_DEP_DIR)/,$(EDITOR_TEST_SRCS)))))

CLI_RELEASE_SUBDIRECTORES:=$(sort $(patsubst %/,%,$(dir $(addprefix $(CLI_RELEASE_OBJ_DIR)/,$(CLI_SRCS))))) $(sort $(patsubst %/,%,$(dir $(addprefix $(CLI_RELEASE_DEP_DIR)/,$(CLI_SRCS)))))
CLI_DEBUG_SUBDIRECTORES:=$(sort $(patsubst %/,%,$(dir $(addprefix $(CLI_DEBUG_OBJ_DIR)/,$(CLI_SRCS))))) $(sort $(patsubst %/,%,$(dir $(addprefix $(CLI_DEBUG_DEP_DIR)/,$(CLI_SRCS)))))
CLI_TEST_SUBDIRECTORES:=$(sort $(patsubst %/,%,$(dir $(addprefix $(CLI_TEST_EXE_DIR)/,$(CLI_TEST_SRCS))))) $(sort $(patsubst %/,%,$(dir $(addprefix $(CLI_TEST_DEP_DIR)/,$(CLI_TEST_SRCS)))))

LIBRARY_RELEASE_SUBDIRECTORES:=$(sort $(patsubst %/,%,$(dir $(addprefix $(LIBRARY_RELEASE_OBJ_DIR)/,$(LIBRARY_SRCS))))) $(sort $(patsubst %/,%,$(dir $(addprefix $(LIBRARY_RELEASE_DEP_DIR)/,$(LIBRARY_SRCS)))))
LIBRARY_DEBUG_SUBDIRECTORES:=$(sort $(patsubst %/,%,$(dir $(addprefix $(LIBRARY_DEBUG_OBJ_DIR)/,$(LIBRARY_SRCS))))) $(sort $(patsubst %/,%,$(dir $(addprefix $(LIBRARY_DEBUG_DEP_DIR)/,$(LIBRARY_SRCS)))))
LIBRARY_TEST_SUBDIRECTORES:=$(sort $(patsubst %/,%,$(dir $(addprefix $(LIBRARY_TEST_EXE_DIR)/,$(LIBRARY_TEST_SRCS))))) $(sort $(patsubst %/,%,$(dir $(addprefix $(LIBRARY_TEST_DEP_DIR)/,$(LIBRARY_TEST_SRCS)))))

UTIL_RELEASE_SUBDIRECTORES:=$(sort $(patsubst %/,%,$(dir $(addprefix $(UTIL_RELEASE_OBJ_DIR)/,$(UTIL_SRCS))))) $(sort $(patsubst %/,%,$(dir $(addprefix $(UTIL_RELEASE_DEP_DIR)/,$(UTIL_SRCS)))))
UTIL_DEBUG_SUBDIRECTORES:=$(sort $(patsubst %/,%,$(dir $(addprefix $(UTIL_DEBUG_OBJ_DIR)/,$(UTIL_SRCS))))) $(sort $(patsubst %/,%,$(dir $(addprefix $(UTIL_DEBUG_DEP_DIR)/,$(UTIL_SRCS)))))
UTIL_TEST_SUBDIRECTORES:=$(sort $(patsubst %/,%,$(dir $(addprefix $(UTIL_TEST_EXE_DIR)/,$(UTIL_TEST_SRCS))))) $(sort $(patsubst %/,%,$(dir $(addprefix $(UTIL_TEST_DEP_DIR)/,$(UTIL_TEST_SRCS)))))

# Declare phony targets (targets without a corresponding file)
.PHONY: all clean help all-debug all-debug-notests all-release all-build-tests all-test editor-all editor-debug editor-debug-notests editor-release editor-build-tests editor-test editor-run editor-run-release cli-all cli-debug cli-debug-notests cli-release cli-build-tests cli-test cli-run cli-run-release library-all library-debug library-debug-notests library-release library-build-tests library-test util-all util-debug util-debug-notests util-release util-build-tests util-test

# Define Phony Targets

all: all-debug all-release
all-debug: editor-debug cli-debug library-debug util-debug
all-debug-notests: editor-debug-notests cli-debug-notests library-debug-notests util-debug-notests
all-release: editor-release cli-release library-release util-release
all-build-tests: editor-build-tests cli-build-tests library-build-tests util-build-tests
all-test: editor-test cli-test library-test util-test
editor-all: editor-debug editor-release
cli-all: cli-debug cli-release
library-all: library-debug library-release
util-all: util-debug util-release

editor-debug: editor-debug-notests editor-build-tests
editor-debug-notests: $(EDITOR_DEBUG_EXE)
editor-release: $(EDITOR_RELEASE_EXE)
editor-build-tests: $(EDITOR_TEST_EXES)
editor-test: editor-build-tests
	@echo "Not yet implemented, please run tests manually for now."
editor-run: $(EDITOR_DEBUG_EXE)
	LD_LIBRARY_PATH=$(LIBRARY_DEBUG_DIR) $(EDITOR_DEBUG_EXE)
editor-run-release: $(EDITOR_RELEASE_EXE)
	LD_LIBRARY_PATH=$(LIBRARY_RELEASE_DIR) $(EDITOR_RELEASE_EXE)

cli-debug: cli-debug-notests cli-build-tests
cli-debug-notests: $(CLI_DEBUG_EXE)
cli-release: $(CLI_RELEASE_EXE)
cli-build-tests: $(CLI_TEST_EXES)
cli-test: cli-build-tests
	@echo "Not yet implemented, please run tests manually for now."
cli-run: $(EDITOR_DEBUG_EXE)
	LD_LIBRARY_PATH=$(LIBRARY_DEBUG_DIR) $(CLI_DEBUG_EXE)
cli-run-release: $(EDITOR_RELEASE_EXE)
	LD_LIBRARY_PATH=$(LIBRARY_RELEASE_DIR) $(CLI_RELEASE_EXE)

library-debug: library-debug-notests library-build-tests
library-debug-notests: $(LIBRARY_DEBUG_LIB)
library-release: $(LIBRARY_RELEASE_LIB)
library-build-tests: $(LIBRARY_TEST_EXES)
library-test: library-build-tests
	@echo "Not yet implemented, please run tests manually for now."

util-debug: util-debug-notests util-build-tests
util-debug-notests: $(UTIL_DEBUG_OBJS)
util-release: $(UTIL_RELEASE_OBJS)
util-build-tests: $(UTIL_TEST_EXES)
util-test: util-build-tests
	@echo "Not yet implemented, please run tests manually for now."

# Delete all build files
clean:
	$(CLEAN)

# Help page
help:
	@echo "GNU make is a build system that builds targets, which are usually files. GNU make looks at dependencies and file update timestamps so it will only rebuild when necessary. The following extra non-file build targets have been defined in the Makefile for this project:"
	@echo ""
	@echo "all - Build all targets"
	@echo "clean - Delete all build outputs"
	@echo "help - Display this help page"
	@echo "all-debug - Build the all debug targets"
	@echo "all-debug-notests - Build the all debug executables without building tests"
	@echo "all-release - Build all release targets"
	@echo "all-build-tests - Build all tests without running them"
	@echo "all-test - Build and run all tests"
	@echo "editor-all - Build all parasheet-editor targets"
	@echo "editor-debug - Build all parasheet-editor debug targets"
	@echo "editor-debug-notests - Build the parasheet-editor debug executable without building tests"
	@echo "editor-release - Build all parasheet-editor release targets"
	@echo "editor-build-tests - Build parasheet-editor tests without running them"
	@echo "editor-test - Build and run all parasheet-editor tests"
	@echo "editor-run - Build and run the debug version of parasheet-editor"
	@echo "editor-run-release - Build and run the debug version of parasheet-editor"
	@echo "cli-all - Build all parasheet-cli targets"
	@echo "cli-debug - Build all parasheet-cli debug targets"
	@echo "cli-debug-notests - Build the parasheet-cli debug executable without building tests"
	@echo "cli-release - Build all parasheet-cli release targets"
	@echo "cli-build-tests - Build parasheet-cli tests without running them"
	@echo "cli-test - Build and run all parasheet-cli tests"
	@echo "cli-run - Build and run the debug version of parasheet-cli"
	@echo "cli-run-release - Build and run the debug version of parasheet-cli"
	@echo "library-all - Build all libparasheet targets"
	@echo "library-debug - Build all libparasheet debug targets"
	@echo "library-debug-notests - Build debug libparasheet without building tests"
	@echo "library-release - Build all libparasheet release targets"
	@echo "library-build-tests - Build libparasheet tests without running them"
	@echo "library-test - Build and run all libparasheet tests"
	@echo "util-all - Build all util targets"
	@echo "util-debug - Build all util debug targets"
	@echo "util-debug-notests - Build all util debug object files without building tests"
	@echo "util-release - Build all util release targets"
	@echo "util-build-tests - Build util tests without running them"
	@echo "util-test - Build and run all util tests"

# Create all neccessary directories
$(EDITOR_RELEASE_SUBDIRECTORES):
	$(MKDIR)
$(EDITOR_DEBUG_SUBDIRECTORES):
	$(MKDIR)
$(EDITOR_TEST_SUBDIRECTORES):
	$(MKDIR)

$(CLI_RELEASE_SUBDIRECTORES):
	$(MKDIR)
$(CLI_DEBUG_SUBDIRECTORES):
	$(MKDIR)
$(CLI_TEST_SUBDIRECTORES):
	$(MKDIR)

$(LIBRARY_RELEASE_SUBDIRECTORES):
	$(MKDIR)
$(LIBRARY_DEBUG_SUBDIRECTORES):
	$(MKDIR)
$(LIBRARY_TEST_SUBDIRECTORES):
	$(MKDIR)

$(UTIL_RELEASE_SUBDIRECTORES):
	$(MKDIR)
$(UTIL_DEBUG_SUBDIRECTORES):
	$(MKDIR)
$(UTIL_TEST_SUBDIRECTORES):
	$(MKDIR)


# Much of the following is from https://make.mad-scientist.net/papers/advanced-auto-dependency-generation

# Link executables and libaries
$(EDITOR_RELEASE_EXE): $(EDITOR_RELEASE_OBJS) $(UTIL_RELEASE_OBJS) $(LIBRARY_RELEASE_LIB)
	$(CC) $(call ospath,$(EDITOR_RELEASE_OBJS) $(UTIL_RELEASE_OBJS)) -o $(call ospath,$@) $(EDITOR_RELEASE_LDFLAGS)
$(EDITOR_DEBUG_EXE): $(EDITOR_DEBUG_OBJS) $(UTIL_DEBUG_OBJS) $(LIBRARY_DEBUG_LIB)
	$(CC) $(call ospath,$(EDITOR_DEBUG_OBJS) $(UTIL_DEBUG_OBJS)) -o $(call ospath,$@) $(EDITOR_DEBUG_LDFLAGS)

$(CLI_RELEASE_EXE): $(CLI_RELEASE_OBJS) $(UTIL_RELEASE_OBJS) $(LIBRARY_RELEASE_LIB)
	$(CC) $(call ospath,$(CLI_RELEASE_OBJS) $(UTIL_RELEASE_OBJS)) -o $(call ospath,$@) $(CLI_RELEASE_LDFLAGS)
$(CLI_DEBUG_EXE): $(CLI_DEBUG_OBJS) $(UTIL_DEBUG_OBJS) $(LIBRARY_DEBUG_LIB)
	$(CC) $(call ospath,$(CLI_DEBUG_OBJS) $(UTIL_DEBUG_OBJS)) -o $(call ospath,$@) $(CLI_DEBUG_LDFLAGS)

$(LIBRARY_RELEASE_LIB): $(LIBRARY_RELEASE_OBJS) $(UTIL_RELEASE_OBJS)
	$(CC) $(call ospath,$(LIBRARY_RELEASE_OBJS) $(UTIL_RELEASE_OBJS)) -shared -o $(call ospath,$@) $(LIBRARY_RELEASE_LDFLAGS)
$(LIBRARY_DEBUG_LIB): $(LIBRARY_DEBUG_OBJS) $(UTIL_DEBUG_OBJS)
	$(CC) $(call ospath,$(LIBRARY_DEBUG_OBJS) $(UTIL_DEBUG_OBJS)) -shared -o $(call ospath,$@) $(LIBRARY_DEBUG_LDFLAGS)

# Compile and link test executables
$(EDITOR_TEST_EXE_DIR)/%: $(EDITOR_TEST_SRC_DIR)/%$(SOURCE_FILE_EXTENSION) $(filter-out main.o,$(EDITOR_DEBUG_OBJS)) $(UTIL_DEBUG_OBJS) $(LIBRARY_DEBUG_LIB) | $(EDITOR_TEST_SUBDIRECTORES)
	$(CC) -MT $(call ospath,$@) -MMD -MP -MF $(call ospath,$(EDITOR_TEST_DEP_DIR)/$*.d) $(EDITOR_TEST_CFLAGS) $(call ospath,$(addprefix -I,$(EDITOR_INCLUDE_DIRS))) $(call ospath,$< $(filter-out %main.o,$(EDITOR_DEBUG_OBJS)) $(UTIL_DEBUG_OBJS)) -o $(call ospath,$@) $(EDITOR_TEST_LDFLAGS)

$(CLI_TEST_EXE_DIR)/%: $(CLI_TEST_SRC_DIR)/%$(SOURCE_FILE_EXTENSION) $(filter-out main.o,$(CLI_DEBUG_OBJS)) $(UTIL_DEBUG_OBJS) $(LIBRARY_DEBUG_LIB) | $(CLI_TEST_SUBDIRECTORES)
	$(CC) -MT $(call ospath,$@) -MMD -MP -MF $(call ospath,$(CLI_TEST_DEP_DIR)/$*.d) $(CLI_TEST_CFLAGS) $(call ospath,$(addprefix -I,$(CLI_INCLUDE_DIRS))) $(call ospath,$< $(filter-out %main.o,$(CLI_DEBUG_OBJS)) $(UTIL_DEBUG_OBJS)) -o $(call ospath,$@) $(CLI_TEST_LDFLAGS)

$(LIBRARY_TEST_EXE_DIR)/%: $(LIBRARY_TEST_SRC_DIR)/%$(SOURCE_FILE_EXTENSION) $(filter-out main.o,$(LIBRARY_DEBUG_OBJS)) $(UTIL_DEBUG_OBJS) | $(LIBRARY_TEST_SUBDIRECTORES)
	$(CC) -MT $(call ospath,$@) -MMD -MP -MF $(call ospath,$(LIBRARY_TEST_DEP_DIR)/$*.d) $(LIBRARY_TEST_CFLAGS) $(call ospath,$(addprefix -I,$(LIBRARY_INCLUDE_DIRS))) $(call ospath,$< $(UTIL_DEBUG_OBJS)) -o $(call ospath,$@) $(LIBRARY_TEST_LDFLAGS)

$(UTIL_TEST_EXE_DIR)/%: $(UTIL_TEST_SRC_DIR)/%$(SOURCE_FILE_EXTENSION) $(filter-out main.o,$(UTIL_DEBUG_OBJS)) $(UTIL_DEBUG_OBJS) | $(UTIL_TEST_SUBDIRECTORES)
	$(CC) -MT $(call ospath,$@) -MMD -MP -MF $(call ospath,$(UTIL_TEST_DEP_DIR)/$*.d) $(UTIL_TEST_CFLAGS) $(call ospath,$(addprefix -I,$(UTIL_INCLUDE_DIRS))) $(call ospath,$< $(UTIL_DEBUG_OBJS)) -o $(call ospath,$@) $(UTIL_TEST_LDFLAGS)

# Compile all object files
$(EDITOR_RELEASE_OBJ_DIR)/%.o: $(EDITOR_SRC_DIR)/%$(SOURCE_FILE_EXTENSION) $(EDITOR_RELEASE_DEP_DIR)/%.d $(EDITOR_INCLUDE_DIRS) | $(EDITOR_RELEASE_SUBDIRECTORES)
	$(CC) -MT $(call ospath,$@) -MMD -MP -MF $(call ospath,$(EDITOR_RELEASE_DEP_DIR)/$*.d) $(EDITOR_RELEASE_CFLAGS) $(call ospath,$(addprefix -I,$(EDITOR_INCLUDE_DIRS))) -c -o $(call ospath,$@) $(call ospath,$<)
$(EDITOR_DEBUG_OBJ_DIR)/%.o: $(EDITOR_SRC_DIR)/%$(SOURCE_FILE_EXTENSION) $(EDITOR_DEBUG_DEP_DIR)/%.d $(EDITOR_INCLUDE_DIRS) | $(EDITOR_DEBUG_SUBDIRECTORES)
	$(CC) -MT $(call ospath,$@) -MMD -MP -MF $(call ospath,$(EDITOR_DEBUG_DEP_DIR)/$*.d) $(EDITOR_DEBUG_CFLAGS) $(call ospath,$(addprefix -I,$(EDITOR_INCLUDE_DIRS))) -c -o $(call ospath,$@) $(call ospath,$<)

$(CLI_RELEASE_OBJ_DIR)/%.o: $(CLI_SRC_DIR)/%$(SOURCE_FILE_EXTENSION) $(CLI_RELEASE_DEP_DIR)/%.d $(CLI_INCLUDE_DIRS) | $(CLI_RELEASE_SUBDIRECTORES)
	$(CC) -MT $(call ospath,$@) -MMD -MP -MF $(call ospath,$(CLI_RELEASE_DEP_DIR)/$*.d) $(CLI_RELEASE_CFLAGS) $(call ospath,$(addprefix -I,$(CLI_INCLUDE_DIRS))) -c -o $(call ospath,$@) $(call ospath,$<)
$(CLI_DEBUG_OBJ_DIR)/%.o: $(CLI_SRC_DIR)/%$(SOURCE_FILE_EXTENSION) $(CLI_DEBUG_DEP_DIR)/%.d $(CLI_INCLUDE_DIRS) | $(CLI_DEBUG_SUBDIRECTORES)
	$(CC) -MT $(call ospath,$@) -MMD -MP -MF $(call ospath,$(CLI_DEBUG_DEP_DIR)/$*.d) $(CLI_DEBUG_CFLAGS) $(call ospath,$(addprefix -I,$(CLI_INCLUDE_DIRS))) -c -o $(call ospath,$@) $(call ospath,$<)

$(LIBRARY_RELEASE_OBJ_DIR)/%.o: $(LIBRARY_SRC_DIR)/%$(SOURCE_FILE_EXTENSION) $(LIBRARY_RELEASE_DEP_DIR)/%.d $(LIBRARY_INCLUDE_DIRS) | $(LIBRARY_RELEASE_SUBDIRECTORES)
	$(CC) -MT $(call ospath,$@) -MMD -MP -MF $(call ospath,$(LIBRARY_RELEASE_DEP_DIR)/$*.d) $(LIBRARY_RELEASE_CFLAGS) $(call ospath,$(addprefix -I,$(LIBRARY_INCLUDE_DIRS))) -c -o $(call ospath,$@) $(call ospath,$<)
$(LIBRARY_DEBUG_OBJ_DIR)/%.o: $(LIBRARY_SRC_DIR)/%$(SOURCE_FILE_EXTENSION) $(LIBRARY_DEBUG_DEP_DIR)/%.d $(LIBRARY_INCLUDE_DIRS) | $(LIBRARY_DEBUG_SUBDIRECTORES)
	$(CC) -MT $(call ospath,$@) -MMD -MP -MF $(call ospath,$(LIBRARY_DEBUG_DEP_DIR)/$*.d) $(LIBRARY_DEBUG_CFLAGS) $(call ospath,$(addprefix -I,$(LIBRARY_INCLUDE_DIRS))) -c -o $(call ospath,$@) $(call ospath,$<)

$(UTIL_RELEASE_OBJ_DIR)/%.o: $(UTIL_SRC_DIR)/%$(SOURCE_FILE_EXTENSION) $(UTIL_RELEASE_DEP_DIR)/%.d $(UTIL_INCLUDE_DIRS) | $(UTIL_RELEASE_SUBDIRECTORES)
	$(CC) -MT $(call ospath,$@) -MMD -MP -MF $(call ospath,$(UTIL_RELEASE_DEP_DIR)/$*.d) $(UTIL_RELEASE_CFLAGS) $(call ospath,$(addprefix -I,$(UTIL_INCLUDE_DIRS))) -c -o $(call ospath,$@) $(call ospath,$<)
$(UTIL_DEBUG_OBJ_DIR)/%.o: $(UTIL_SRC_DIR)/%$(SOURCE_FILE_EXTENSION) $(UTIL_DEBUG_DEP_DIR)/%.d $(UTIL_INCLUDE_DIRS) | $(UTIL_DEBUG_SUBDIRECTORES)
	$(CC) -MT $(call ospath,$@) -MMD -MP -MF $(call ospath,$(UTIL_DEBUG_DEP_DIR)/$*.d) $(UTIL_DEBUG_CFLAGS) $(call ospath,$(addprefix -I,$(UTIL_INCLUDE_DIRS))) -c -o $(call ospath,$@) $(call ospath,$<)


# Include all dependency files (so make knows which files to recompile when a header file is updated)
DEPFILES:= $(EDITOR_SRCS:%$(SOURCE_FILE_EXTENSION)=$(EDITOR_RELEASE_DEP_DIR)/%.d) $(EDITOR_SRCS:%$(SOURCE_FILE_EXTENSION)=$(EDITOR_DEBUG_DEP_DIR)/%.d) $(EDITOR_TEST_SRCS:%$(SOURCE_FILE_EXTENSION)=$(EDITOR_TEST_DEP_DIR)/%.d) $(CLI_SRCS:%$(SOURCE_FILE_EXTENSION)=$(CLI_RELEASE_DEP_DIR)/%.d) $(CLI_SRCS:%$(SOURCE_FILE_EXTENSION)=$(CLI_DEBUG_DEP_DIR)/%.d) $(CLI_TEST_SRCS:%$(SOURCE_FILE_EXTENSION)=$(CLI_TEST_DEP_DIR)/%.d) $(LIBRARY_SRCS:%$(SOURCE_FILE_EXTENSION)=$(LIBRARY_RELEASE_DEP_DIR)/%.d) $(LIBRARY_SRCS:%$(SOURCE_FILE_EXTENSION)=$(LIBRARY_DEBUG_DEP_DIR)/%.d) $(LIBRARY_TEST_SRCS:%$(SOURCE_FILE_EXTENSION)=$(LIBRARY_TEST_DEP_DIR)/%.d) $(UTIL_SRCS:%$(SOURCE_FILE_EXTENSION)=$(UTIL_RELEASE_DEP_DIR)/%.d) $(UTIL_SRCS:%$(SOURCE_FILE_EXTENSION)=$(UTIL_DEBUG_DEP_DIR)/%.d) $(UTIL_TEST_SRCS:%$(SOURCE_FILE_EXTENSION)=$(UTIL_TEST_DEP_DIR)/%.d)
$(DEPFILES):
include $(wildcard $(DEPFILES))
