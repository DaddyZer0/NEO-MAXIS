# Building NEO-MAXIS

## Requirements

- CMake 3.20+
- C++17 compiler
- MinGW-w64 or MSVC for Windows DLL builds
- Compatible sc2kfix source checkout containing `mods/sc2kfix.h`

## Native tests (Linux)

Run from the repository root:

    cmake -S . -B build-tests -DNEOMAXIS_BUILD_TESTS=ON
    cmake --build build-tests --parallel 2
    ctest --test-dir build-tests --output-on-failure

## Windows DLL (MinGW-w64)

Replace `/path/to/sc2kfix-source` with the actual sc2kfix checkout path:

    cmake -S . -B build-windows -DCMAKE_TOOLCHAIN_FILE=cmake/mingw32-toolchain.cmake -DSC2KFIX_SOURCE_DIR=/path/to/sc2kfix-source -DCMAKE_BUILD_TYPE=Release
    cmake --build build-windows --parallel 2

Expected output: `build-windows/neomaxis.dll`.

## Windows DLL (MSVC)

Run from a Visual Studio Developer PowerShell:

    cmake -S . -B build-windows -A Win32 -DSC2KFIX_SOURCE_DIR="C:\path\to\sc2kfix-source"
    cmake --build build-windows --config Release

Expected output: `build-windows\Release\neomaxis.dll`.

## Notes

- Native tests do not require the original game or sc2kfix.
- DLL builds require a compatible sc2kfix checkout.
- Private local sc2kfix modifications are not included here.
- Do not commit generated build directories or original game files.
