default:
    @just --list --unsorted

# Development workflow (build/)

# configures the build system for development mode
configure:
    cmake --preset debug

# builds the program in development mode
build:
    cmake --build --preset debug

# runs all tests
test: build
    ctest --preset debug

# runs tests containing the specified TAG
test-tag TAG: build
    ./build/tests/unit-tests {{TAG}} 

# runs the program
run: build
    ./build/src/gui/monkey-moore-gui

# runs the program with gbd attached for debugging
run-debugger: build
    gdb -q -ex "set debuginfod enabled on" -ex "set confirm off" -ex run --args ./build/src/gui/monkey-moore-gui

# Release workflow (build-release/)

# configures the build system for release mode
configure-release:
    cmake --preset release

# builds the program in release mode
build-release:
    cmake --build --preset release

# runs all tests against release artifacts
test-release: build-release
    ctest --preset release

# runs benchmarks against release artifacts
benchmark: build-release
    ./build-release/benchmarks/benchmarks --benchmark_time_unit=ms

# runs the program in release mode
run-release: build-release
    ./build-release/src/gui/monkey-moore-gui

# Utils

# cleans all temporary directories
clean:
    rm -rf build build-release