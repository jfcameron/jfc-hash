# jfc-hash
Stable, non-cryptographic hashing library. The same input gives the same output on every supported platform, and compiler.

An integer is hashed at its own width: the fixed-width types, `char` and `int` are the same everywhere, while `long`, `std::size_t`, `std::ptrdiff_t` and `wchar_t` are not, since their widths differ between platforms. `splitmix64` gives the same bits everywhere, but the standard library's distributions do not; `unit_double` and `unit_float` do.

API documentation: https://jfcameron.github.io/jfc-hash/

## Getting Started
Install cmake 3.21 or greater, and a c++20 compatible compiler 
```
# initialize the tree
git submodule update --init --recursive

# generate the script to build the library on linux using gcc
cmake --preset linux-gcc

# build the library
cmake --build --preset linux-gcc

# run the test suite
ctest --preset linux-gcc
```
See `CMakePresets.json` for the full list of supported platforms and compilers. See `.github/workflows/ci.yml` to see exactly how CI runners build it on different platforms.
