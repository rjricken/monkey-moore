cmake_minimum_required(VERSION 3.10)
project(MonkeyMoore)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(external)
add_subdirectory(src/core)
add_subdirectory(src/gui)