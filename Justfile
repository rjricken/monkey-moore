alias b := build

configure:
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug

build:
    cmake --build build

test:
    ctest --test-dir build --output-on-failure

run: build
    GTK_THEME=Adwaita:light ./build/src/gui/monkey-moore-gui