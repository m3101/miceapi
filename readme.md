# miceapi - Multi-Mouse API

An API for abstracting linux devices and making multiple simultaneous mice/pointer handling easier.

This project was built and is maintained by a single person, thus, it hasn't been thoroughly tested (except for very basic use cases), so please report any issues and suggestions using this platform.

## Documentation

* [Python module Documentation (At GitHub repo Wiki)][pydocs]
* [C API Documentation (At GitHub repo Wiki)][cdocs]

## How to run the examples

### C
Follow the [build instuctions](#Building-with-the-C-API) for the chosen example file.

### Python
Install the package using pip ([here's the command](#Installing-the-python-module)) and run the example script with python3.

## Usage instructions
* [Python (via pip)](#Installing-the-python-module)
* [C (build instructions)](#Building-with-the-C-API)

### Installing the python module

Run the following command:

`pip install miceapi`

### Building with the C API

* Pre-requisites
  * Linux environment
  * gcc

To make the commands easier to interpret, assume `$MICEPATH` is set to the path to the project root. You may either set it (`MICEPATH=<path>`) before running the commands or just manually replace it with the correct path.

Let's suppose you want to build an executable from a file named `example.c` (which contains `#include "$MICEPATH/src/miceapi_main.h"` and/or `#include "$MICEPATH/src/miceapi_events.h"` directives).

* Method A - From shared library (Recommended)
  * If you haven't yet built the project, at the root directory of the project, run `make build`. This will generate a folder containing the file `libmiceapi.so`.
  * If you don't know what to do with this file, keep following these steps.
  * Run `gcc -L$MICEPATH/lib -Wl,-rpath=$MICEPATH/lib -o example example.c -lmiceapi` (replacing `example.c` with the actual file you want to build and `example` with the output executable name). The same options should work for g++ too.

* Method B - From source (Not Recommended)
  * Run `gcc -o example example.c $MICEPATH/src/miceapi_main.c $MICEPATH/src/miceapi_events.c` (replacing `example.c` with the actual file you want to build and `example` with the output executable name).

## Important note

One might experience problems with permissions when running programs that use this API. The easiest (yet kind of unhealthy and potentially dangerous if you don't know what you are doing) way of dealing with this is running python/your executable with superuser privileges (`sudo`). You may also configure access permissions for the specific files you are trying to access.


---

By Amélia O. F. da S.

^_^

[pydocs]: https://github.com/m3101/miceapi/wiki/Python-module-documentation
[cdocs]: https://github.com/m3101/miceapi/wiki/C-API-Documentation