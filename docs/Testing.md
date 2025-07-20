# Testing this project

Write tests and place them in a subdirectory of the `tests/` directory. Then you can build and run these tests via GNU make. The make targets for running tests are `all-test`, `editor-test`, `cli-test`, `library-test`, and `util-test`. For more details, see the building page.

You can also invoke the test runner directly, when your working directory is the root directory of the project. The test runner takes as input a list of files to run, then runs them. For more information, see the help page.
```
This is a small test runner to run test files.
Usage: build/tests/test-runner <OPTIONS> <FILES>
Possible options are:
 --help - Display this help message.
 -e - Make memory leaks result in a program error (status code 2)
 -l - Disable checking for leaks with valgrind
 -v - Verbose; print test output
 -w - Extra verbose; print test & valgrind output
```
