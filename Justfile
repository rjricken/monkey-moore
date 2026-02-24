alias b := build

# Development workflow (build/)
configure:
    cmake --preset debug

build:
    cmake --build --preset debug

test: build
    ctest --preset debug

test-tag TAG: build
    ./build/tests/unit-tests {{TAG}} 

run: build
    ./build/src/gui/monkey-moore-gui

# runs the program with gbd attached for debugging
run-debugger: build
    gdb -q -ex "set debuginfod enabled on" -ex "set confirm off" -ex run --args ./build/src/gui/monkey-moore-gui

# Release workflow (build-release/)
configure-release:
    cmake --preset release

build-release:
    cmake --build --preset release

benchmark: build-release
    ./build-release/benchmarks/benchmarks --benchmark_time_unit=ms

run-release: build-release
    ./build-release/src/gui/monkey-moore-gui

# Utils
clean:
    rm -rf build build-release