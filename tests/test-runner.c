#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

typedef enum TestState {
	PASS,
	FAIL,
	LEAK,
	MISSING_FILES,
	OTHER_ERROR
} TestState;

typedef struct CmdOpts {
	bool checkLeaks;
	bool leakError;
	bool verbose;
	bool extraVerbose;
} CmdOpts;

void printHeader() {
	printf("+----------------------------------------------------+\n"
		   "|                  Running Tests...                  |\n"
		   "+----------------------------------------------------+\n");
}

void printFooter(TestState state) {
	switch (state) {
	case PASS:
		printf("+----------------------------------------------------+\n"
			   "|                 All tests passed!                  |\n"
			   "+----------------------------------------------------+\n");
		return;
	case LEAK:
		printf("+----------------------------------------------------+\n"
			   "|               Memory leaks detected!               |\n"
			   "+----------------------------------------------------+\n");
		return;
	case FAIL:
		printf("+----------------------------------------------------+\n"
			   "|                  Tests Failed D:                   |\n"
			   "+----------------------------------------------------+\n");
		return;
	case MISSING_FILES:
		printf("+----------------------------------------------------+\n"
			   "|                Test files missing!                 |\n"
			   "+----------------------------------------------------+\n");
		return;
	default:
		return;
	}
}

void printPass(int spaces, bool fullline) {
	if (fullline) {
		printf("\033[1m#======================= \033[32mPASS\033[0m\033[1m "
			   "=======================#\033[0m\n");
	} else {
		printf("%*s\n", spaces, "\033[1m\033[32mPASS\033[0m");
	}
}

void printLeak(int spaces, bool fullline) {
	if (fullline) {
		printf("\033[1m#======================= \033[33mLEAK\033[0m\033[1m "
			   "=======================#\033[0m\n");
	} else {
		printf("%*s\n", spaces, "\033[1m\033[33mLEAK\033[0m");
	}
}

void printFail(int spaces, bool fullline) {
	if (fullline) {
		printf("\033[1m#======================= \033[31mFAIL\033[0m\033[1m "
			   "=======================#\033[0m\n");
	} else {
		printf("%*s\n", spaces, "\033[1m\033[31mFAIL\033[0m");
	}
}

void printMissing(int spaces, bool fullline) {
	if (fullline) {
		printf("\033[1m#===================== \033[35mMISSING\033[0m\033[1m "
			   "======================#\033[0m\n");
	} else {
		printf("%*s\n", spaces, "\033[1m\033[35mMISSING\033[0m");
	}
}

int main(int argc, char** argv) {
	// Initialize options
	CmdOpts opts = {.checkLeaks = true,
					.leakError = false,
					.verbose = false,
					.extraVerbose = false};

	// Initialize test file list
	char** testFiles;
	int numTestfiles;

	// Parse Arguments
	char* argFiles[argc];
	int numArgFiles = 0;
	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (argv[i][1] == '-') {
				if (strcmp(argv[i], "--help") == 0) {
					printf(
						"This is a small test runner to run test files.\n"
						"Usage: %s <OPTIONS> <FILES>\n"
						"Possible options are:\n"
						" --help - Display this help message.\n"
						" -e - Make memory leaks result in a program error "
						"(status code %d)\n"
						" -l - Disable checking for leaks with valgrind\n"
						" -v - Verbose; print test output\n"
						" -w - Extra verbose; print test & valgrind output\n",
						argv[0], LEAK);
					exit(EXIT_SUCCESS);
				} else {
					fprintf(stderr, "Unrecognized flag: %s\n", argv[i]);
					exit(OTHER_ERROR);
				}
			} else {
				for (int j = 1; argv[i][j] != '\0'; j++) {
					switch (argv[i][j]) {
					case 'l': // No leak checking
						opts.checkLeaks = false;
						break;
					case 'e': // Leaks are errors in return value
						opts.leakError = true;
						break;
					case 'v': // Verbose; print test output
						opts.verbose = true;
						break;
					case 'w': // Extra Verbose; print Valgrind output
						opts.verbose = true;
						opts.extraVerbose = true;
						break;
					default: // Unrecognized option
						fprintf(stderr, "Unrecognized option: -%c\n",
								argv[i][j]);
						exit(OTHER_ERROR);
					}
				}
			}
		} else {
			argFiles[numArgFiles++] = argv[i];
		}
	}

	// Get test files
	if (numArgFiles > 0) {
		testFiles = &(argFiles[0]);
		numTestfiles = numArgFiles;
	} else {
#ifdef TEST_FILES
		char* defaultFiles[] = TEST_FILES;
		testFiles = &(defaultFiles[0]);
		numTestfiles = sizeof(defaultFiles) / sizeof(defaultFiles[0]);
#else
		fprintf(stderr, "No test files provided! Please provide test files as "
						"command line arguments.\n");
#endif
	}

	// Set necessary libraries
#ifdef TEST_LIBRARIES
	char* currentEnv = getenv("LD_LIBRARY_PATH");
	if (currentEnv == NULL) {
		setenv("LD_LIBRARY_PATH", TEST_LIBRARIES, 0);
	} else {
		size_t len = strlen(currentEnv);
		char newEnv[sizeof(TEST_LIBRARIES) + len + 1];
		sprintf(newEnv, "%s;%s", TEST_LIBRARIES, currentEnv);
		setenv("LD_LIBRARY_PATH", newEnv, 1);
	}
#endif

	TestState totalState = MISSING_FILES;

	int nullfd = open("/dev/null", O_WRONLY);

	printHeader();

	pid_t curr_pid;
	int statusCode;

	// Check if valgrind is installed
	if (opts.checkLeaks) {
		curr_pid = fork();
		if (curr_pid < 0) {
			exit(OTHER_ERROR);
		} else if (curr_pid == 0) {
			dup2(nullfd, 1);
			dup2(nullfd, 2);

			execlp("valgrind", "valgrind", "--help", NULL);

			if (errno == ENOENT) {
				exit(127);
			} else {
				exit(OTHER_ERROR);
			}
		}
		waitpid(curr_pid, &statusCode, 0);
		if (statusCode == 32512) {
			fprintf(stderr, "Warning: valgrind not found. Running tests "
							"without leak detection.\n");
			opts.checkLeaks = false;
		} else if (statusCode != 0) {
			exit(OTHER_ERROR);
		}
	}

	for (int i = 0; i < numTestfiles; i++) {
		char* filepath1 = strdup(testFiles[i]);
		char* file = basename(filepath1);
		char* filepath2 = strdup(testFiles[i]);
		char* directory = dirname(filepath2);
		directory = basename(directory);
		printf("- \033[1m%s/%s\033[0m...%c", directory, file,
			   opts.verbose ? '\n' : ' ');
		fflush(stdout); // flush stdout so user can see file that is running
		int baselength = strlen(directory) + strlen(file) + 8;
		free(filepath1);
		free(filepath2);
		curr_pid = fork();
		if (curr_pid < 0) {
			exit(OTHER_ERROR);
		} else if (curr_pid == 0) {
			if (!opts.verbose) {
				dup2(nullfd, 1);
				dup2(nullfd, 2);
			}

			if (opts.checkLeaks) {
				if (opts.extraVerbose) {
					execlp("valgrind", "valgrind", "--error-exitcode=66",
						   "--leak-check=full", "--", testFiles[i], NULL);
				} else {
					execlp("valgrind", "valgrind", "-q", "--error-exitcode=66",
						   "--leak-check=full", "--", testFiles[i], NULL);
				}
				exit(OTHER_ERROR);
			} else {
				execl(testFiles[i], testFiles[i], NULL);
				if (errno == ENOENT) {
					exit(127); // 127 indicates missing file
				} else {
					exit(OTHER_ERROR);
				}
			}
		}
		waitpid(curr_pid, &statusCode, 0);

		if (statusCode == EXIT_SUCCESS) {
			if (totalState == MISSING_FILES) {
				totalState = PASS;
			}
			printPass(68 - baselength, opts.verbose);
		} else if (statusCode == 32512) {
			// I'm not quite sure how 127 gets mangled into 32512, but whatever:
			// it works
			printMissing(68 - baselength, opts.verbose);
		} else if (opts.checkLeaks && statusCode == 16896) {
			// I'm not quite sure how 66 gets mangled into 16896, but whatever:
			// it works
			if (totalState == MISSING_FILES || totalState == PASS) {
				totalState = LEAK;
			}
			printLeak(68 - baselength, opts.verbose);
		} else {
			totalState = FAIL;
			printFail(68 - baselength, opts.verbose);
		}
	}

	printFooter(totalState);

	if (opts.leakError) {
		return totalState;
	} else {
		return totalState == LEAK ? PASS : totalState;
	}
}
