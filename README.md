vtrace
======

An R package for collecting information about how vectors and data frames are
used in R programs.

See the [`vtrace-analysis`](https://github.com/reactorlabs/vtrace-analysis)
"meta" repository for details on how to develop and use this package.

Architecture
------------

  * `R/` - R source code
    * `R/tracer.R` - the main application code, in R
  * `src/` - C++ source code
    * `src/callbacks.{h,cpp}` - the main application code, in C++

### Boilerplate

  * `src/init.c` - Registers native functions so they can be called from R
  * `src/r_callbacks.{h,cpp}` - Wrapper functions that return R pointers to C++
    functions

### Libraries

  * `src/lib/picosha2.h` - [PicoSHA2](https://github.com/okdshin/PicoSHA2) is a
    small, header-only, C++ library for generating SHA256 hashes
