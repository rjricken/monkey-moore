cmake_minimum_required(VERSION 3.19)
project(MonkeyMoore)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

add_subdirectory(src/core)
add_subdirectory(src/gui)

option(BUILD_TESTING "Build unit tests" OFF)
option(BUILD_BENCHMARKS "Build performance benchmarks" OFF)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()