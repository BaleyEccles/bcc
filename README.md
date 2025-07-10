# Baley's C Compiler
## Build
Just run the following command in any Linux terminal.
```console
$ make 
```
This will produce a executable called `bcc`, which is the compiler.

## Tests
A suite tests have been developed in the `tests/` directory, to run these tests run the following command.
```console
$ make test
``
This will execute the `test.sh` script which will enter each test in the `tests/` directory and run it.
Each of the tests will indicate if they passed or failed. If a test fails it will highligt the differences in output using the `diff` command.


## Dependencies
This is a Linux project, it is expected that you compile and run everything on Linux. The project is compiled using GCC, other compilers have not been tested. 
Common Unix tooling is required to run the test suite, namely `find` and `diff`, these should be present on most Linux systems.
